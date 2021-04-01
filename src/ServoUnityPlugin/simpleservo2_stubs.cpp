//
// This allows testing the plugin without any libsimpleservo2 implementation.
// Just define the macro SIMPLESERVO2_STUBS.
//

#include "simpleservo2.h"

#if SIMPLESERVO2_STUBS

static CHostCallbacks s_callbacks = {nullptr};
static bool s_shutdown_requested = false;

void change_visibility(bool visible)
{
}

void clear_cache(void)
{
}

void click(float x, float y)
{
}

void deinit(void)
{
}

void fill_gl_texture(uint32_t tex_id, int32_t tex_width, int32_t tex_height)
{
}

CPref get_pref(const char *key)
{
	static const CPref p = {};
	return p;
}

const bool *get_pref_as_bool(const void *ptr)
{
	static const bool b = false;
	return &b;
}

const double *get_pref_as_float(const void *ptr)
{
	static const double d = 0.0;
	return &d;
}

const int64_t *get_pref_as_int(const void *ptr)
{
	static const int64_t i = 0;
	return &i;
}

const char *get_pref_as_str(const void *ptr)
{
	static const char s[] = "";
	return s;
}

CPrefList get_prefs(void)
{
	static const CPrefList pl = {};
	return pl;
}

void go_back(void)
{
}

void go_forward(void)
{
}

void ime_dismissed(void)
{
}

void init_with_egl(CInitOptions opts, void (*wakeup)(void), CHostCallbacks callbacks)
{
	s_callbacks = callbacks;
}

void init_with_gl(CInitOptions opts, void (*wakeup)(void), CHostCallbacks callbacks)
{
	s_callbacks = callbacks;
}

bool is_uri_valid(const char *url)
{
	return true;
}

void key_down(uint32_t key_code, CKeyType key_type)
{
}

void key_up(uint32_t key_code, CKeyType key_type)
{
}

bool load_uri(const char *url)
{
	if (s_callbacks.on_url_changed) {
		(*s_callbacks.on_url_changed)(url);
	}
	return true;
}

void media_session_action(CMediaSessionActionType action)
{
}

void mouse_down(float x, float y, CMouseButton button)
{
}

void mouse_move(float x, float y)
{
}

void mouse_up(float x, float y, CMouseButton button)
{
}

void on_context_menu_closed(CContextMenuResult result, uint32_t item)
{
}

void perform_updates(void)
{
	if (s_shutdown_requested) {
		if (s_callbacks.on_shutdown_complete) (*s_callbacks.on_shutdown_complete)();
		s_shutdown_requested = false;
	}
}

void pinchzoom(float factor, int32_t x, int32_t y)
{
}

void pinchzoom_end(float factor, int32_t x, int32_t y)
{
}

void pinchzoom_start(float factor, int32_t x, int32_t y)
{
}

void refresh(void)
{
}

void register_panic_handler(void (*on_panic)(const char*))
{
}

void reload(void)
{
}

void request_shutdown(void)
{
}

void reset_all_prefs(void)
{
}

bool reset_pref(const char *key)
{
	return true;
}

void resize(int32_t width, int32_t height)
{
}

void scroll(int32_t dx, int32_t dy, int32_t x, int32_t y)
{
}

void scroll_end(int32_t dx, int32_t dy, int32_t x, int32_t y)
{
}

void scroll_start(int32_t dx, int32_t dy, int32_t x, int32_t y)
{
}

// The returned string is not freed. This will leak.
const char *servo_version(void)
{
	static const char vs[] = "SIMPLESERVO2_STUBS";
	return vs;
}

void set_batch_mode(bool batch)
{
}

bool set_bool_pref(const char *key, bool value)
{
	return true;
}

bool set_float_pref(const char *key, double value)
{
	return true;
}

bool set_int_pref(const char *key, int64_t value)
{
	return true;
}

bool set_str_pref(const char *key, const char *value)
{
	return true;
}

void stop(void)
{
}

void touch_cancel(float x, float y, int32_t pointer_id)
{
}

void touch_down(float x, float y, int32_t pointer_id)
{
}

void touch_move(float x, float y, int32_t pointer_id)
{
}

void touch_up(float x, float y, int32_t pointer_id)
{
}

#endif // SIMPLESERVO2_STUBS