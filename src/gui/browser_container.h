#ifndef BROWSER_CONTAINER_H
#define BROWSER_CONTAINER_H

#include <litehtml.h>
#include <string>
#include <EASTL/string.h>
#include <map>

struct cached_image {
    int width;
    int height;
    int channels;
    unsigned char* data;
};

class browser_container : public litehtml::document_container {
public:
    browser_container();
    virtual ~browser_container();

    virtual litehtml::uint_ptr create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override;
    virtual void delete_font(litehtml::uint_ptr hFont) override;
    virtual litehtml::pixel_t text_width(const char* text, litehtml::uint_ptr hFont) override;
    virtual void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
    virtual litehtml::pixel_t pt_to_px(float pt) const override;
    virtual litehtml::pixel_t get_default_font_size() const override;
    virtual const char* get_default_font_name() const override;
    virtual void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
    virtual void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
    virtual void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override;
    virtual void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) override;
    virtual void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override;
    virtual void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) override;
    virtual void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) override;
    virtual void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) override;
    virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;

    virtual void set_caption(const char* caption) override;
    virtual void set_base_url(const char* base_url) override;
    virtual void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
    virtual void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
    virtual void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;
    virtual void set_cursor(const char* cursor) override;
    virtual void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
    virtual void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
    virtual void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
    virtual void del_clip() override;
    virtual void get_viewport(litehtml::position& viewport) const override;
    virtual litehtml::element::ptr create_element(const char* tag_name, const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc) override;

    virtual void get_media_features(litehtml::media_features& media) const override;
    virtual void get_language(litehtml::string& language, litehtml::string& culture) const override;

    const char* get_base_url() const { return m_base_url.c_str(); }
    void set_client_rect(int width, int height);

private:
    litehtml::position m_viewport;
    eastl::string m_base_url;
    std::map<std::string, cached_image> m_image_cache;
};

#endif
