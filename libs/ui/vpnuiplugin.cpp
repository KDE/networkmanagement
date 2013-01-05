#include <KLocale>

#include "vpnuiplugin.h"

class VpnUiPlugin::Private
{
public:
    SettingWidget * widget;
};

VpnUiPlugin::VpnUiPlugin(QObject * parent) : QObject(parent), d(new Private)
{
    mError = NoError;
}

VpnUiPlugin::~VpnUiPlugin()
{
    delete d;
}

KDialog::ButtonCodes VpnUiPlugin::suggestAuthDialogButtons()
{
    return KDialog::Ok | KDialog::Cancel;
}

VpnUiPlugin::ErrorType VpnUiPlugin::lastError() {
    return mError;
}

QString VpnUiPlugin::lastErrorMessage() {
    switch (mError) {
        case NoError:
            mErrorMessage = "";
            break;
        case NotImplemented:
            return i18nc("Error message in VPN import/export dialog", "Operation not supported for this VPN type.");
            break;
        case Error:
            break;
    }
    return mErrorMessage;
}

// vim: sw=4 sts=4 et tw=100
