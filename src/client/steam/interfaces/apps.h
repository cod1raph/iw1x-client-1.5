#pragma once

namespace steam
{
    class apps
    {
    public:
        ~apps() = default;

        apps()
        {
        }

        virtual bool BIsSubscribedApp(/*unsigned int appID*/);
    };
}