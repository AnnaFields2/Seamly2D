/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file   vsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vsettings.h"

#include <QDir>
#include <QLocale>
#include <QMetaType>
#include <QPrinter>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QVariant>
#include <QPrinterInfo>
#include <QtDebug>

#include "../vmisc/def.h"
#include "../vmisc/vmath.h"

Q_DECLARE_METATYPE(QMarginsF)

namespace
{
const QString settingConfigurationLabelLanguage = QStringLiteral("configuration/label_language");

const QString settingPathsPattern = QStringLiteral("paths/pattern");
const QString settingPathsLayout  = QStringLiteral("paths/layout");

const QString settingPatternGraphicalOutput = QStringLiteral("pattern/graphicalOutput");

const QString settingCommunityServer       = QStringLiteral("community/server");
const QString settingCommunityServerSecure = QStringLiteral("community/serverSecure");
const QString settingCommunityUseProxy     = QStringLiteral("community/useProxy");
const QString settingCommunityProxyAddress = QStringLiteral("community/proxyAddress");
const QString settingCommunityProxyPort    = QStringLiteral("community/proxyPort");
const QString settingCommunityProxyUser    = QStringLiteral("community/proxyUser");
const QString settingCommunityProxyPass    = QStringLiteral("community/proxyPass");
const QString settingCommunityUsername     = QStringLiteral("community/username");
const QString settingCommunitySavePassword = QStringLiteral("community/savePassword");
const QString settingCommunityUserPassword = QStringLiteral("community/userpassword");

const QString settingLayoutWidth            = QStringLiteral("layout/width");
const QString settingLayoutSorting          = QStringLiteral("layout/sorting");
const QString settingLayoutPaperHeight      = QStringLiteral("layout/paperHeight");
const QString settingLayoutPaperWidth       = QStringLiteral("layout/paperWidth");
const QString settingLayoutShift            = QStringLiteral("layout/shift");
const QString settingLayoutRotate           = QStringLiteral("layout/Rotate");
const QString settingLayoutRotationIncrease = QStringLiteral("layout/rotationIncrease");
const QString settingLayoutAutoCrop         = QStringLiteral("layout/autoCrop");
const QString settingLayoutSaveLength       = QStringLiteral("layout/saveLength");
const QString settingLayoutUnitePages       = QStringLiteral("layout/unitePages");
const QString settingFields                 = QStringLiteral("layout/fields");
const QString settingIgnoreFields           = QStringLiteral("layout/ignoreFields");
const QString settingStripOptimization      = QStringLiteral("layout/stripOptimization");
const QString settingMultiplier             = QStringLiteral("layout/multiplier");
const QString settingTextAsPaths            = QStringLiteral("layout/textAsPaths");

const QString settingTiledPDFMargins        = QStringLiteral("tiledPDF/margins");
const QString settingTiledPDFPaperHeight    = QStringLiteral("tiledPDF/paperHeight");
const QString settingTiledPDFPaperWidth     = QStringLiteral("tiledPDF/paperWidth");
const QString settingTiledPDFOrientation    = QStringLiteral("tiledPDF/orientation");
}

//---------------------------------------------------------------------------------------------------------------------
VSettings::VSettings(Format format, Scope scope, const QString &organization, const QString &application,
                     QObject *parent)
    :VCommonSettings(format, scope, organization, application, parent)
{
    qRegisterMetaTypeStreamOperators<QMarginsF>("QMarginsF");
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetLabelLanguage() const
{
    return value(settingConfigurationLabelLanguage, QLocale().bcp47Name()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLabelLanguage(const QString &value)
{
    setValue(settingConfigurationLabelLanguage, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::getDefaultPatternPath()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("patterns");
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::getPatternPath() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), this->applicationName());
    return settings.value(settingPathsPattern, getDefaultPatternPath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetPathPattern(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), this->applicationName());
    settings.setValue(settingPathsPattern, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::getDefaultLayoutPath()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("layouts");
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::getLayoutPath() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), this->applicationName());
    return settings.value(settingPathsLayout, getDefaultLayoutPath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetPathLayout(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), this->applicationName());
    settings.setValue(settingPathsLayout, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetGraphicalOutput() const
{
    return value(settingPatternGraphicalOutput, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetGraphicalOutput(const bool &value)
{
    setValue(settingPatternGraphicalOutput, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetServer() const
{
    return value(settingCommunityServer, "community.seamly2d-project.org").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetServer(const QString &value)
{
    setValue(settingCommunityServer, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetServerSecure() const
{
    return value(settingCommunityServerSecure, 0).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetServerSecure(const bool &value)
{
    setValue(settingCommunityServerSecure, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetProxy() const
{
    return value(settingCommunityUseProxy, 0).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetProxy(const bool &value)
{
    setValue(settingCommunityUseProxy, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetProxyAddress() const
{
    return value(settingCommunityProxyAddress).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetProxyAddress(const QString &value)
{
    setValue(settingCommunityProxyAddress, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetProxyPort() const
{
    return value(settingCommunityProxyPort).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetProxyPort(const QString &value)
{
    setValue(settingCommunityProxyPort, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetProxyUser() const
{
    return value(settingCommunityProxyUser).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetProxyUser(const QString &value)
{
    setValue(settingCommunityProxyUser, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetProxyPass() const
{
    return value(settingCommunityProxyPass).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetProxyPass(const QString &value)
{
    setValue(settingCommunityProxyPass, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetUsername() const
{
    return value(settingCommunityUsername).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetUsername(const QString &value)
{
    setValue(settingCommunityUsername, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetSavePassword() const
{
    return value(settingCommunitySavePassword, 0).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetSavePassword(const bool &value)
{
    setValue(settingCommunitySavePassword, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VSettings::GetUserPassword() const
{
    return value(settingCommunityUserPassword).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetUserPassword(const QString &value)
{
    setValue(settingCommunityUserPassword, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSettings::GetLayoutPaperHeight() const
{
    const qreal def = UnitConvertor(1189/*A0*/, Unit::Mm, Unit::Px);
    bool ok = false;
    const qreal height = value(settingLayoutPaperHeight, def).toDouble(&ok);
    if (ok)
    {
        return height;
    }
    else
    {
        return def;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutPaperHeight(qreal value)
{
    setValue(settingLayoutPaperHeight, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSettings::GetLayoutPaperWidth() const
{
    const qreal def = UnitConvertor(841/*A0*/, Unit::Mm, Unit::Px);
    bool ok = false;
    const qreal width = value(settingLayoutPaperWidth, def).toDouble(&ok);
    if (ok)
    {
        return width;
    }
    else
    {
        return def;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutPaperWidth(qreal value)
{
    setValue(settingLayoutPaperWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSettings::GetLayoutShift() const
{
    const qreal def = GetDefLayoutShift();
    bool ok = false;
    const qreal shift = value(settingLayoutShift, def).toDouble(&ok);
    if (ok)
    {
        return shift;
    }
    else
    {
        return def;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSettings::GetDefLayoutShift()
{
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutShift(qreal value)
{
    setValue(settingLayoutShift, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSettings::GetLayoutWidth() const
{
    const qreal def = GetDefLayoutWidth();
    bool ok = false;
    const qreal lWidth = value(settingLayoutWidth, def).toDouble(&ok);
    if (ok)
    {
        return lWidth;
    }
    else
    {
        return def;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSettings::GetDefLayoutWidth()
{
    return UnitConvertor(2.5, Unit::Mm, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutWidth(qreal value)
{
    setValue(settingLayoutWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VSettings::GetFields(const QMarginsF &def) const
{
    const QVariant val = value(settingFields, QVariant::fromValue(def));
    if (val.canConvert<QMarginsF>())
    {
        return val.value<QMarginsF>();
    }
    return def;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetFields(const QMarginsF &value)
{
    setValue(settingFields, QVariant::fromValue(value));
}

//---------------------------------------------------------------------------------------------------------------------
Cases VSettings::GetLayoutGroup() const
{
    const Cases def = GetDefLayoutGroup();
    bool ok = false;
    const int g = value(settingLayoutSorting, static_cast<int>(def)).toInt(&ok);
    if (ok)
    {
        if (g >= static_cast<int>(Cases::UnknownCase))
        {
            return def;
        }
        else
        {
            return static_cast<Cases>(g);
        }
    }
    else
    {
        return def;
    }
}

//---------------------------------------------------------------------------------------------------------------------
Cases VSettings::GetDefLayoutGroup()
{
    return Cases::CaseDesc;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutGroup(const Cases &value)
{
    setValue(settingLayoutSorting, static_cast<int>(value));
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetLayoutRotate() const
{
    return value(settingLayoutRotate, GetDefLayoutRotate()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefLayoutRotate()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutRotate(bool value)
{
    setValue(settingLayoutRotate, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VSettings::GetLayoutRotationIncrease() const
{
    const int def = GetDefLayoutRotationIncrease();
    bool ok = false;
    const int r = value(settingLayoutRotationIncrease, def).toInt(&ok);
    if (ok)
    {
        if (not (r >= 1 && r <= 180 && 360 % r == 0))
        {
            return def;
        }
        else
        {
            return r;
        }
    }
    else
    {
        return def;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VSettings::GetDefLayoutRotationIncrease()
{
    return 180;/*degree*/
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutRotationIncrease(int value)
{
    setValue(settingLayoutRotationIncrease, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetLayoutAutoCrop() const
{
    return value(settingLayoutAutoCrop, GetDefLayoutAutoCrop()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefLayoutAutoCrop()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutAutoCrop(bool value)
{
    setValue(settingLayoutAutoCrop, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetLayoutSaveLength() const
{
    return value(settingLayoutSaveLength, GetDefLayoutSaveLength()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefLayoutSaveLength()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutSaveLength(bool value)
{
    setValue(settingLayoutSaveLength, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetLayoutUnitePages() const
{
    return value(settingLayoutUnitePages, GetDefLayoutUnitePages()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefLayoutUnitePages()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetLayoutUnitePages(bool value)
{
    setValue(settingLayoutUnitePages, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetIgnoreAllFields() const
{
    return value(settingIgnoreFields, GetDefIgnoreAllFields()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefIgnoreAllFields()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetIgnoreAllFields(bool value)
{
    setValue(settingIgnoreFields, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetStripOptimization() const
{
    return value(settingStripOptimization, GetDefStripOptimization()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefStripOptimization()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetStripOptimization(bool value)
{
    setValue(settingStripOptimization, value);
}

//---------------------------------------------------------------------------------------------------------------------
quint8 VSettings::GetMultiplier() const
{
    return static_cast<quint8>(value(settingMultiplier, GetDefMultiplier()).toUInt());
}

//---------------------------------------------------------------------------------------------------------------------
quint8 VSettings::GetDefMultiplier()
{
    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::SetMultiplier(quint8 value)
{
    setValue(settingMultiplier, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetTextAsPaths() const
{
    return value(settingTextAsPaths, GetDefTextAsPaths()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VSettings::GetDefTextAsPaths()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::setTextAsPaths(bool value)
{
    setValue(settingTextAsPaths, value);
}

// settings for the tiled PDFs
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSettings::GetTiledPDFMargins returns the tiled pdf margins in the given unit. When the setting is
 * called for the first time, the 4 default margins are 10mm.
 * @param unit
 * @return
 */
QMarginsF VSettings::GetTiledPDFMargins(const Unit &unit) const
{
    // default value is 10mm. We save the margins in mm in the setting.
    const QMarginsF def = QMarginsF(10,10,10,10);

    const QVariant val = value(settingTiledPDFMargins, QVariant::fromValue(def));

    if (val.canConvert<QMarginsF>())
    {
        return UnitConvertor(val.value<QMarginsF>(), Unit::Mm, unit);
    }
    return UnitConvertor(def, Unit::Mm, unit);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSettings::setTiledPDFMargins sets the setting tiled pdf margins to the given value.
 * @param value the margins to save
 * @param unit the unit in which are the value. Necessary because we save the values
 * internaly as mm so there is conversion beeing made.
 */
void VSettings::setTiledPDFMargins(const QMarginsF &value, const Unit &unit)
{
    QMarginsF margins = UnitConvertor(value, unit, Unit::Mm);

    setValue(settingTiledPDFMargins, QVariant::fromValue(margins));
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSettings::getTiledPDFPaperHeight returns the paper height of tiled pdf in the desired unit.
 * @param unit the unit to return the value to (internally it's saved as mm)
 * @return
 */
qreal VSettings::getTiledPDFPaperHeight(const Unit &unit) const
{
    const qreal def = 297 /*A4*/;
    bool ok = false;
    const qreal height = value(settingTiledPDFPaperHeight, def).toDouble(&ok);
    if (ok)
    {
        return UnitConvertor(height, Unit::Mm, unit);
    }
    else
    {
        return UnitConvertor(def, Unit::Mm, unit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSettings::setTiledPDFPaperHeight sets the tiled pdf paper height
 * @param value in Mm
 * @param unit unit of the given value
 */
void VSettings::setTiledPDFPaperHeight(qreal value, const Unit &unit)
{
    setValue(settingTiledPDFPaperHeight, UnitConvertor(value, unit, Unit::Mm));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSettings::getTiledPDFPaperWidth returns the paper height of tiled pdf in the desired unit.
 * @param unit the unit to return the value to (internally it's saved as mm)
 * @return
 */
qreal VSettings::getTiledPDFPaperWidth(const Unit &unit) const
{

    const qreal def = 210 /*A4*/;
    bool ok = false;
    const qreal width = value(settingTiledPDFPaperWidth, def).toDouble(&ok);

    if (ok)
    {
        return UnitConvertor(width, Unit::Mm, unit);
    }
    else
    {
        return UnitConvertor(def, Unit::Mm, unit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSettings::setTiledPDFPaperWidth sets the tiled pdf paper width
 * @param unit unit of the given value
 * @param value in Mm
 */
void VSettings::setTiledPDFPaperWidth(qreal value, const Unit &unit)
{
    setValue(settingTiledPDFPaperWidth, UnitConvertor(value,unit, Unit::Mm));
}

//---------------------------------------------------------------------------------------------------------------------
PageOrientation VSettings::getTiledPDFOrientation() const
{
    bool defaultValue = static_cast<bool>(PageOrientation::Portrait);

    bool result = value(settingTiledPDFOrientation, defaultValue).toBool();

    return static_cast<PageOrientation>(result);
}

//---------------------------------------------------------------------------------------------------------------------
void VSettings::setTiledPDFOrientation(PageOrientation value)
{
    bool orientation = static_cast<bool> (value);

    setValue(settingTiledPDFOrientation, orientation);
}
