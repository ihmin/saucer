#pragma once

#include "webview.impl.hpp"

#include "gtk.utils.hpp"
#include "wkg.scheme.impl.hpp"

#include <vector>

#include <webkit/webkit.h>

namespace saucer
{
    using script_ptr          = utils::ref_ptr<WebKitUserScript, webkit_user_script_ref, webkit_user_script_unref>;
    using content_manager_ptr = utils::g_object_ptr<WebKitUserContentManager>;

    struct wkg_script
    {
        script_ptr ref;
        bool clearable;
    };

    struct webview::impl::native
    {
        WebKitWebView *web_view;

      public:
        gulong msg_received;
        content_manager_ptr manager;

      public:
        bool context_menu{true};

      public:
        std::uint64_t id_counter{0};
        std::unordered_map<std::uint64_t, wkg_script> scripts;

      public:
        bool dom_loaded{false};
        std::vector<std::string> pending;

      public:
        utils::g_object_ptr<WebKitSettings> settings;

      public:
        template <event>
        void setup(impl *);

      public:
        static gboolean on_context(WebKitWebView *, WebKitContextMenu *, WebKitHitTestResult *, impl *);

      public:
        static void on_message(WebKitWebView *, JSCValue *, impl *);
        static void on_load(WebKitWebView *, WebKitLoadEvent, impl *);

      public:
        static void on_click(GtkGestureClick *, gint, gdouble, gdouble, impl *);
        static void on_release(GtkGestureClick *, gdouble, gdouble, guint, GdkEventSequence *, impl *);

      public:
        static WebKitSettings *make_settings(const options &);
        static inline std::unordered_map<std::string, std::unique_ptr<scheme::handler>> schemes;
    };
} // namespace saucer
