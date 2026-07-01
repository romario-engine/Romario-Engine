#include "ui/image_view.h"

image_view::image_view()
{
	image = nullptr;
	set_identity_matrix(&transform);
	fm.content_size.assign_lambda([this] (uint32 viewport_width, uint32 viewport_height) -> vector<uint32, 2>
		{
			vector<float32, 3> p1(0.0f, 0.0f, 0.0f), p2(this->image->width, 0.0f, 0.0f),
				p3(this->image->width, this->image->height, 0.0f), p4(0.0f, this->image->height, 0.0f);
			p1 = p1 * transform;
			p2 = p2 * transform;
			p3 = p3 * transform;
			p4 = p4 * transform;
			vector<float32, 2> p_min(min(p1.x, p2.x, p3.x, p4.x), min(p1.y, p2.y, p3.y, p4.y)),
				p_max(max(p1.x, p2.x, p3.x, p4.x), max(p1.y, p2.y, p3.y, p4.y));
			return vector<uint32, 2>(uint32(ceilf(p_max.x - p_min.x)), uint32(ceilf(p_max.y - p_min.y)));
		});
	fm.render.assign_lambda([this] (graphics_displayer *gd, bitmap *bmp) -> void
		{
			rectangle<int32> content_viewport = this->fm.content_viewport();
			if(!this->fm.visible || content_viewport.extent.x <= 0 || content_viewport.extent.y <= 0 || image == nullptr) return;
			vector<float32, 3> p1(0.0f, 0.0f, 0.0f), p2(this->image->width, 0.0f, 0.0f),
				p3(this->image->width, this->image->height, 0.0f), p4(0.0f, this->image->height, 0.0f);
			p1 = p1 * transform;
			p2 = p2 * transform;
			p3 = p3 * transform;
			p4 = p4 * transform;
			vector<float32, 2> p_min(min(p1.x, p2.x, p3.x, p4.x), min(p1.y, p2.y, p3.y, p4.y)),
				p_max(max(p1.x, p2.x, p3.x, p4.x), max(p1.y, p2.y, p3.y, p4.y));
			float32 scale, width, height;
			if(float32(content_viewport.extent.x) / float32(content_viewport.extent.y)
				>= float32(p_max.x - p_min.x) / float32(p_max.y - p_min.y))
				scale = float32(content_viewport.extent.y) / float32(p_max.y - p_min.y);
			else scale = float32(content_viewport.extent.x) / float32(p_max.x - p_min.x);
			width = floorf(float32(p_max.x - p_min.x) * scale);
			height = floorf(float32(p_max.y - p_min.y) * scale);
			vector<float32, 2> p(
				floorf(float32(content_viewport.position.x) + 0.5f * (float32(content_viewport.extent.x) - width)),
				floorf(float32(content_viewport.position.y) + 0.5f * (float32(content_viewport.extent.y) - height)));
			gd->brush.switch_bitmap(this->image, this->transform * translating_matrix(-p_min.x, -p_min.y)
				* scaling_matrix(scale, scale, vector<float32, 2>(0.0f, 0.0f)) * translating_matrix(p.x, p.y));
			gd->fill_rect(rectangle<int32>(vector<int32, 2>(int32(p.x), int32(p.y)), vector<int32, 2>(int32(width), int32(height))), bmp);
		});
}
