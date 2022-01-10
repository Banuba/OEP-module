#pragma once

#include <memory>

namespace bnb::oep::interfaces
{
    class render_context;
}

using render_context_sptr = std::shared_ptr<bnb::oep::interfaces::render_context>;

namespace bnb::oep::interfaces
{

    class render_context
    {
    public:
        /**
         * Create the rendering context.
         *
         * @return - shared pointer to the rendering context
         *
         * @example bnb::oep::interfaces::render_context::create()
         */
        static render_context_sptr create();

        virtual ~render_context() = default;

        /**
         * Create and initialize a rendering context. Should be called in offscreen render target.
         *
         * @example create_context()
         */
        virtual void create_context() = 0;

        /**
         * Make the rendering context active on the current thread. Should be called in
         * offscreen render target.
         *
         * @example activate()
         */
        virtual void activate() = 0;

        /**
         * Turn off rendering context on current thread. Should be called in offscreen render target.
         *
         * @example deactivate()
         */
        virtual void deactivate() = 0;

        /**
         * Delete a rendering context. Should be called in offscreen render target.
         *
         * @example delete_context()
         */
        virtual void delete_context() = 0;

        /**
         * Returns a raw pointer to the rendering context. This pointer is interpreted differently
         * on each platform.
         *
         * @example get_sharing_context();
         */
        virtual void* get_sharing_context() = 0;
    }; /* class render_context  INTERFACE */

} /* namespace bnb::oep::interfaces */
