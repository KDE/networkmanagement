#include "vpnuiplugin.h"

class VpnUiPlugin::Private
{
public:
    SettingWidget * widget;
};

VpnUiPlugin::VpnUiPlugin() : d(new Private)
{

}

VpnUiPlugin::~VpnUiPlugin()
{
    delete d->widget;
    delete d;
}

// vim: sw=4 sts=4 et tw=100
