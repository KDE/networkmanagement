#include "vpnuiplugin.h"

class VpnUiPlugin::Private
{
public:
    SettingWidget * widget;
};

VpnUiPlugin::VpnUiPlugin(QObject * parent) : QObject(parent), d(new Private)
{

}

VpnUiPlugin::~VpnUiPlugin()
{
    delete d;
}

KDialog::ButtonCodes VpnUiPlugin::suggestAuthDialogButtons()
{
    return KDialog::Ok | KDialog::Cancel;
}

// vim: sw=4 sts=4 et tw=100
