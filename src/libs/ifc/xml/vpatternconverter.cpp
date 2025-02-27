/******************************************************************************
 *   @file   vpatternconverter.h
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

/**************************************************************************
 **
 **  @file   vpatternconverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 12, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vpatternconverter.h"

#include "abstract_converter.h"
#include "../exception/vexception.h"
#include "../exception/vexceptionemptyparameter.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/def.h"
#include "../vmisc/logging.h"

#include <QDir>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QFileInfo>
#include <QLatin1String>
#include <QList>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <algorithm>

class QDomElement;

Q_LOGGING_CATEGORY(PatternConverter, "patternConverter")

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VPatternConverter::PatternMinVerStr = QStringLiteral("0.1.0");
const QString VPatternConverter::PatternMaxVerStr = QStringLiteral("0.6.8");
const QString VPatternConverter::CurrentSchema    = QStringLiteral("://schema/pattern/v0.6.8.xsd");

//VPatternConverter::PatternMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
//VPatternConverter::PatternMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

// The list of all string we use for conversion
// Better to use global variables because repeating QStringLiteral blows up code size
static const QString strUnit                      = QStringLiteral("unit");
static const QString strVersion                   = QStringLiteral("version");
static const QString strName                      = QStringLiteral("name");
static const QString strBase                      = QStringLiteral("base");
static const QString strFormula                   = QStringLiteral("formula");
static const QString strId                        = QStringLiteral("id");
static const QString strKGrowth                   = QStringLiteral("kgrowth");
static const QString strKSize                     = QStringLiteral("ksize");
static const QString strPoint                     = QStringLiteral("point");
static const QString strLength                    = QStringLiteral("length");
static const QString strAngle                     = QStringLiteral("angle");
static const QString strC1Radius                  = QStringLiteral("c1Radius");
static const QString strC2Radius                  = QStringLiteral("c2Radius");
static const QString strCRadius                   = QStringLiteral("cRadius");
static const QString strArc                       = QStringLiteral("arc");
static const QString strAngle1                    = QStringLiteral("angle1");
static const QString strAngle2                    = QStringLiteral("angle2");
static const QString strRadius                    = QStringLiteral("radius");
static const QString strPathPoint                 = QStringLiteral("pathPoint");
static const QString strKAsm1                     = QStringLiteral("kAsm1");
static const QString strKAsm2                     = QStringLiteral("kAsm2");
static const QString strPath                      = QStringLiteral("path");
static const QString strType                      = QStringLiteral("type");
static const QString strCutArc                    = QStringLiteral("cutArc");
static const QString strSpline                    = QStringLiteral("spline");
static const QString strSplinePath                = QStringLiteral("splinePath");
static const QString strCutSpline                 = QStringLiteral("cutSpline");
static const QString strCutSplinePath             = QStringLiteral("cutSplinePath");
static const QString strColor                     = QStringLiteral("color");
static const QString strMeasurements              = QStringLiteral("measurements");
static const QString strIncrement                 = QStringLiteral("increment");
static const QString strIncrements                = QStringLiteral("increments");
static const QString strModeling                  = QStringLiteral("modeling");
static const QString strTools                     = QStringLiteral("tools");
static const QString strIdTool                    = QStringLiteral("idTool");
static const QString strIdObject                  = QStringLiteral("idObject");
static const QString strChildren                  = QStringLiteral("children");
static const QString strChild                     = QStringLiteral("child");
static const QString strPointOfIntersectionCurves = QStringLiteral("pointOfIntersectionCurves");
static const QString strCurveIntersectAxis        = QStringLiteral("curveIntersectAxis");
static const QString strCurve                     = QStringLiteral("curve");
static const QString strCurve1                    = QStringLiteral("curve1");
static const QString strCurve2                    = QStringLiteral("curve2");
static const QString strModelingPath              = QStringLiteral("modelingPath");
static const QString strModelingSpline            = QStringLiteral("modelingSpline");
static const QString strPointFromArcAndTangent    = QStringLiteral("pointFromArcAndTangent");
static const QString strPointOfIntersectionArcs   = QStringLiteral("pointOfIntersectionArcs");
static const QString strFirstArc                  = QStringLiteral("firstArc");
static const QString strSecondArc                 = QStringLiteral("secondArc");
static const QString strDraw                      = QStringLiteral("draw");
static const QString strDraftBlock                = QStringLiteral("draftBlock");
static const QString strDetails                   = QStringLiteral("details");
static const QString strPieces                    = QStringLiteral("pieces");
static const QString strDetail                    = QStringLiteral("detail");
static const QString strPiece                     = QStringLiteral("piece");
static const QString strUnionDetail               = QStringLiteral("unionDetail");
static const QString strUnion                     = QStringLiteral("union");
static const QString strSupplement                = QStringLiteral("supplement");
static const QString strClosed                    = QStringLiteral("closed");
static const QString strWidth                     = QStringLiteral("width");
static const QString strHeight                    = QStringLiteral("height");
static const QString strNode                      = QStringLiteral("node");
static const QString strNodes                     = QStringLiteral("nodes");
static const QString strData                      = QStringLiteral("data");
static const QString strPatternInfo               = QStringLiteral("patternInfo");
static const QString strGrainline                 = QStringLiteral("grainline");
static const QString strReverse                   = QStringLiteral("reverse");
static const QString strMx                        = QStringLiteral("mx");
static const QString strMy                        = QStringLiteral("my");
static const QString strForbidFlipping            = QStringLiteral("forbidFlipping");
static const QString strInLayout                  = QStringLiteral("inLayout");
static const QString strSeamAllowance             = QStringLiteral("seamAllowance");
static const QString strNodeType                  = QStringLiteral("nodeType");
static const QString strDet                       = QStringLiteral("det");
static const QString strTypeObject                = QStringLiteral("typeObject");
static const QString strReadOnly                  = QStringLiteral("readOnly");
static const QString strPatternLabel              = QStringLiteral("patternLabel");
static const QString strImage                     = QStringLiteral("image");
static const QString strAuthor                    = QStringLiteral("author");
static const QString strDescription               = QStringLiteral("description");
static const QString strNotes                     = QStringLiteral("notes");
static const QString strGradation                 = QStringLiteral("gradation");
static const QString strPatternName               = QStringLiteral("patternName");
static const QString strPatternNum                = QStringLiteral("patternNumber");
static const QString strCompanyName               = QStringLiteral("company");
static const QString strCustomerName              = QStringLiteral("customer");
static const QString strLine                      = QStringLiteral("line");
static const QString strText                      = QStringLiteral("text");
static const QString strBold                      = QStringLiteral("bold");
static const QString strItalic                    = QStringLiteral("italic");
static const QString strAlignment                 = QStringLiteral("alignment");
static const QString strFSIncrement               = QStringLiteral("sfIncrement");
static const QString strShowDate                  = QStringLiteral("showDate");
static const QString strShowMeasurements          = QStringLiteral("showMeasurements");
static const QString strSize                      = QStringLiteral("size");
static const QString strMCP                       = QStringLiteral("mcp");
static const QString strLetter                    = QStringLiteral("letter");
static const QString strMaterial                  = QStringLiteral("material");
static const QString strUserDefined               = QStringLiteral("userDef");
static const QString strPlacement                 = QStringLiteral("placement");
static const QString strPassmark                  = QStringLiteral("passmark");
static const QString strPassmarkLine              = QStringLiteral("passmarkLine");
static const QString strPassmarkAngle             = QStringLiteral("passmarkAngle");
static const QString strShowSecondPassmark        = QStringLiteral("showSecondPassmark");
static const QString strNotch                     = QStringLiteral("notch");
static const QString strNotchCount                = QStringLiteral("notchCount");
static const QString strNotchAngle                = QStringLiteral("notchAngle");
static const QString strNotchType                 = QStringLiteral("notchType");
static const QString strNotchSubType              = QStringLiteral("notchSubtype");
static const QString strShowNotch                 = QStringLiteral("showNotch");
static const QString strShowSecond                = QStringLiteral("showSecondNotch");
static const QString strSlitNotch                 = QStringLiteral("slit");
static const QString strVMark                     = QStringLiteral("vMark");
static const QString strV_External                = QStringLiteral("vExternal");
static const QString strTMark                     = QStringLiteral("tMark");
static const QString strT_Notch                   = QStringLiteral("tNotch");
static const QString strHair                      = QStringLiteral("hair");
static const QString strSolidLine                 = QStringLiteral("solidLine");
static const QString strOne                       = QStringLiteral("one");
static const QString strTwo                       = QStringLiteral("two");
static const QString strThree                     = QStringLiteral("three");
static const QString strTypeLine                  = QStringLiteral("typeLine");
static const QString strLineType                  = QStringLiteral("lineType");
static const QString strLineWeight                = QStringLiteral("lineWeight");
static const QString strLineColor                 = QStringLiteral("lineColor");
static const QString strPenStyle                  = QStringLiteral("penStyle");
static const QString strElArc                     = QStringLiteral("elArc");
static const QString strTrue                      = QStringLiteral("true");

static const QString strPin                       = QStringLiteral("pin");
static const QString strPins                      = QStringLiteral("pins");
static const QString strTopLeftPin                = QStringLiteral("topLeftPin");
static const QString strBottomRightPin            = QStringLiteral("bottomRightPin");
static const QString strCenterPin                 = QStringLiteral("centerPin");
static const QString strTopPin                    = QStringLiteral("topPin");
static const QString strBottomPin                 = QStringLiteral("bottomPin");

static const QString strAnchor                    = QStringLiteral("anchor");
static const QString strAnchors                   = QStringLiteral("anchors");
static const QString strTopLeftAnchor             = QStringLiteral("topLeftAnchor");
static const QString strBottomRightAnchor         = QStringLiteral("bottomRightAnchor");
static const QString strCenterAnchor              = QStringLiteral("centerAnchor");
static const QString strTopAnchor                 = QStringLiteral("topAnchor");
static const QString strBottomAnchor              = QStringLiteral("bottomAnchor");

//---------------------------------------------------------------------------------------------------------------------
VPatternConverter::VPatternConverter(const QString &fileName)
    : VAbstractConverter(fileName)
{
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
QString VPatternConverter::XSDSchema(int ver) const
{
    switch (ver)
    {
        case (0x000100):
            return QStringLiteral("://schema/pattern/v0.1.0.xsd");
        case (0x000101):
            return QStringLiteral("://schema/pattern/v0.1.1.xsd");
        case (0x000102):
            return QStringLiteral("://schema/pattern/v0.1.2.xsd");
        case (0x000103):
            return QStringLiteral("://schema/pattern/v0.1.3.xsd");
        case (0x000104):
            return QStringLiteral("://schema/pattern/v0.1.4.xsd");
        case (0x000200):
            return QStringLiteral("://schema/pattern/v0.2.0.xsd");
        case (0x000201):
            return QStringLiteral("://schema/pattern/v0.2.1.xsd");
        case (0x000202):
            return QStringLiteral("://schema/pattern/v0.2.2.xsd");
        case (0x000203):
            return QStringLiteral("://schema/pattern/v0.2.3.xsd");
        case (0x000204):
            return QStringLiteral("://schema/pattern/v0.2.4.xsd");
        case (0x000205):
            return QStringLiteral("://schema/pattern/v0.2.5.xsd");
        case (0x000206):
            return QStringLiteral("://schema/pattern/v0.2.6.xsd");
        case (0x000207):
            return QStringLiteral("://schema/pattern/v0.2.7.xsd");
        case (0x000300):
            return QStringLiteral("://schema/pattern/v0.3.0.xsd");
        case (0x000301):
            return QStringLiteral("://schema/pattern/v0.3.1.xsd");
        case (0x000302):
            return QStringLiteral("://schema/pattern/v0.3.2.xsd");
        case (0x000303):
            return QStringLiteral("://schema/pattern/v0.3.3.xsd");
        case (0x000304):
            return QStringLiteral("://schema/pattern/v0.3.4.xsd");
        case (0x000305):
            return QStringLiteral("://schema/pattern/v0.3.5.xsd");
        case (0x000306):
            return QStringLiteral("://schema/pattern/v0.3.6.xsd");
        case (0x000307):
            return QStringLiteral("://schema/pattern/v0.3.7.xsd");
        case (0x000308):
            return QStringLiteral("://schema/pattern/v0.3.8.xsd");
        case (0x000309):
            return QStringLiteral("://schema/pattern/v0.3.9.xsd");
        case (0x000400):
            return QStringLiteral("://schema/pattern/v0.4.0.xsd");
        case (0x000401):
            return QStringLiteral("://schema/pattern/v0.4.1.xsd");
        case (0x000402):
            return QStringLiteral("://schema/pattern/v0.4.2.xsd");
        case (0x000403):
            return QStringLiteral("://schema/pattern/v0.4.3.xsd");
        case (0x000404):
            return QStringLiteral("://schema/pattern/v0.4.4.xsd");
        case (0x000405):
            return QStringLiteral("://schema/pattern/v0.4.5.xsd");
        case (0x000406):
            return QStringLiteral("://schema/pattern/v0.4.6.xsd");
        case (0x000407):
            return QStringLiteral("://schema/pattern/v0.4.7.xsd");
        case (0x000408):
            return QStringLiteral("://schema/pattern/v0.4.8.xsd");
        case (0x000500):
            return QStringLiteral("://schema/pattern/v0.5.0.xsd");
        case (0x000501):
            return QStringLiteral("://schema/pattern/v0.5.1.xsd");
        case (0x000600):
            return QStringLiteral("://schema/pattern/v0.6.0.xsd");
        case (0x000601):
            return QStringLiteral("://schema/pattern/v0.6.1.xsd");
        case (0x000602):
            return QStringLiteral("://schema/pattern/v0.6.2.xsd");
        case (0x000603):
            return QStringLiteral("://schema/pattern/v0.6.3.xsd");
        case (0x000604):
            return QStringLiteral("://schema/pattern/v0.6.4.xsd");
        case (0x000605):
            return QStringLiteral("://schema/pattern/v0.6.5.xsd");
        case (0x000606):
            return QStringLiteral("://schema/pattern/v0.6.6.xsd");;
        case (0x000607):
            return QStringLiteral("://schema/pattern/v0.6.7.xsd");;
        case (0x000608):
            qCDebug(PatternConverter, "Current schema - ://schema/pattern/v0.6.8.xsd");
            return CurrentSchema;
        default:
            InvalidVersion(ver);
            break;
    }
    return QString();//unreachable code
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (0x000100):
            toVersion0_1_1();
            ValidateXML(XSDSchema(0x000101), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000101):
            toVersion0_1_2();
            ValidateXML(XSDSchema(0x000102), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000102):
            toVersion0_1_3();
            ValidateXML(XSDSchema(0x000103), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000103):
            toVersion0_1_4();
            ValidateXML(XSDSchema(0x000104), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000104):
            toVersion0_2_0();
            ValidateXML(XSDSchema(0x000200), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000200):
            toVersion0_2_1();
            ValidateXML(XSDSchema(0x000201), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000201):
            toVersion0_2_2();
            ValidateXML(XSDSchema(0x000202), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000202):
            toVersion0_2_3();
            ValidateXML(XSDSchema(0x000203), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000203):
            toVersion0_2_4();
            ValidateXML(XSDSchema(0x000204), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000204):
            toVersion0_2_5();
            ValidateXML(XSDSchema(0x000205), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000205):
            toVersion0_2_6();
            ValidateXML(XSDSchema(0x000206), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000206):
            toVersion0_2_7();
            ValidateXML(XSDSchema(0x000207), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000207):
            toVersion0_3_0();
            ValidateXML(XSDSchema(0x000300), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000300):
            toVersion0_3_1();
            ValidateXML(XSDSchema(0x000301), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000301):
            toVersion0_3_2();
            ValidateXML(XSDSchema(0x000302), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000302):
            toVersion0_3_3();
            ValidateXML(XSDSchema(0x000303), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000303):
            toVersion0_3_4();
            ValidateXML(XSDSchema(0x000304), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000304):
            toVersion0_3_5();
            ValidateXML(XSDSchema(0x000305), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000305):
            toVersion0_3_6();
            ValidateXML(XSDSchema(0x000306), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000306):
            toVersion0_3_7();
            ValidateXML(XSDSchema(0x000307), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000307):
            toVersion0_3_8();
            ValidateXML(XSDSchema(0x000308), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000308):
            toVersion0_3_9();
            ValidateXML(XSDSchema(0x000309), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000309):
            toVersion0_4_0();
            ValidateXML(XSDSchema(0x000400), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000400):
            toVersion0_4_1();
            ValidateXML(XSDSchema(0x000401), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000401):
            toVersion0_4_2();
            ValidateXML(XSDSchema(0x000402), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000402):
            toVersion0_4_3();
            ValidateXML(XSDSchema(0x000403), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000403):
            toVersion0_4_4();
            ValidateXML(XSDSchema(0x000404), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000404):
            toVersion0_4_5();
            ValidateXML(XSDSchema(0x000405), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000405):
            toVersion0_4_6();
            ValidateXML(XSDSchema(0x000406), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000406):
            toVersion0_4_7();
            ValidateXML(XSDSchema(0x000407), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000407):
            toVersion0_4_8();
            ValidateXML(XSDSchema(0x000408), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000408):
            toVersion0_5_0();
            ValidateXML(XSDSchema(0x000500), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000500):
            toVersion0_5_1();
            ValidateXML(XSDSchema(0x000501), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000501):
            toVersion0_6_0();
            ValidateXML(XSDSchema(0x000600), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000600):
            toVersion0_6_1();
            ValidateXML(XSDSchema(0x000601), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000601):
            toVersion0_6_2();
            ValidateXML(XSDSchema(0x000602), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000602):
            toVersion0_6_3();
            ValidateXML(XSDSchema(0x000603), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000603):
            toVersion0_6_4();
            ValidateXML(XSDSchema(0x000604), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000604):
            toVersion0_6_5();
            ValidateXML(XSDSchema(0x000605), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000605):
            toVersion0_6_6();
            ValidateXML(XSDSchema(0x000606), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000606):
            toVersion0_6_7();
            ValidateXML(XSDSchema(0x000607), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000607):
            toVersion0_6_8();
            ValidateXML(XSDSchema(0x000608), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000608):
            break;
        default:
            InvalidVersion(m_ver);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(PatternMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPatternConverter::IsReadOnly() const
{
    // Check if attribute readOnly was not changed in file format
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMaxVer == CONVERTER_VERSION_CHECK(0, 6, 8),
                      "Check attribute readOnly.");

    // Possibly in future attribute readOnly will change position etc.
    // For now position is the same for all supported format versions.
    // But don't forget to keep all versions of attribute until we support that format versions

    const QDomElement pattern = documentElement();

    if (pattern.isNull())
    {
        return false;
    }

    return getParameterBool(pattern, strReadOnly, falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_1_1()
{
    // TODO. Delete if minimal supported version is 0.1.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 1),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.1.1"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_1_2()
{
    // TODO. Delete if minimal supported version is 0.1.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 2),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.1.2"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_1_3()
{
    // TODO. Delete if minimal supported version is 0.1.3
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 3),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.1.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_1_4()
{
    // TODO. Delete if minimal supported version is 0.1.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 4),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.1.4"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.0"));
    TagUnitToV0_2_0();
    TagIncrementToV0_2_0();
    ConvertMeasurementsToV0_2_0();
    TagMeasurementsToV0_2_0();//Alwayse last!!!
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_1()
{
    // TODO. Delete if minimal supported version is 0.2.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 1),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.1"));
    ConvertMeasurementsToV0_2_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_2()
{
    // TODO. Delete if minimal supported version is 0.2.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 2),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.2"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_3()
{
    // TODO. Delete if minimal supported version is 0.2.3
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 3),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_4()
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 4),
                      "Time to refactor the code.");

    FixToolUnionToV0_2_4();
    SetVersion(QStringLiteral("0.2.4"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_5()
{
    // TODO. Delete if minimal supported version is 0.2.5
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 5),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.5"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_6()
{
    // TODO. Delete if minimal supported version is 0.2.6
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 6),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.6"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_2_7()
{
    // TODO. Delete if minimal supported version is 0.2.7
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 7),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.7"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 0),
                      "Time to refactor the code.");

    //Cutting path do not create anymore subpaths
    FixCutPoint();
    FixCutPoint();
    SetVersion(QStringLiteral("0.3.0"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 1),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.1"));
    RemoveColorToolCutV0_3_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_2()
{
    // TODO. Delete if minimal supported version is 0.3.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 2),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.2"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_3()
{
    // TODO. Delete if minimal supported version is 0.3.3
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 3),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_4()
{
    // TODO. Delete if minimal supported version is 0.3.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 4),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.4"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_5()
{
    // TODO. Delete if minimal supported version is 0.3.5
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 5),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.5"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_6()
{
    // TODO. Delete if minimal supported version is 0.3.6
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 6),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.6"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_7()
{
    // TODO. Delete if minimal supported version is 0.3.7
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 7),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.7"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_8()
{
    // TODO. Delete if minimal supported version is 0.3.8
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 8),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.8"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_3_9()
{
    // TODO. Delete if minimal supported version is 0.3.9
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 9),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.9"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.0"));
    TagRemoveAttributeTypeObjectInV0_4_0();
    TagDetailToV0_4_0();
    TagUnionDetailsToV0_4_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_1()
{
    // TODO. Delete if minimal supported version is 0.4.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 1),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.1"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_2()
{
    // TODO. Delete if minimal supported version is 0.4.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 2),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.2"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_3()
{
    // TODO. Delete if minimal supported version is 0.4.3
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 3),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_4()
{
    // TODO. Delete if minimal supported version is 0.4.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 4),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.4"));
    LabelTagToV0_4_4(strData);
    LabelTagToV0_4_4(strPatternInfo);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_5()
{
    // TODO. Delete if minimal supported version is 0.4.5
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 5),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.4.5"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_6()
{
    // TODO. Delete if minimal supported version is 0.4.6
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 6),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.4.6"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_7()
{
    // TODO. Delete if minimal supported version is 0.4.7
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 7),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.4.7"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_4_8()
{
    // TODO. Delete if minimal supported version is 0.4.8
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 8),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.4.8"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_5_0()
{
    // TODO. Delete if minimal supported version is 0.5.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 5, 0),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.5.0"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_5_1()
{
    // TODO. Delete if minimal supported version is 0.5.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 5, 1),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.5.1"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_0()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.0"));
    QDomElement label = AddTagPatternLabelV0_5_1();
    PortPatternLabeltoV0_6_0(label);
    PortPieceLabelstoV0_6_0();
    RemoveUnusedTagsV0_6_0();
    Save();
}


//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_1()
{
    // TODO. Delete if minimal supported version is 0.6.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 1),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.6.1"));

    QStringList nodenames = QStringList() << strNode
                                          << strPoint
                                          << strLine
                                          << strPath
                                          << strArc
                                          << strSpline
                                          << strElArc;

    QStringList oldnames = QStringList() << strPassmark
                                         << strPassmarkLine
                                         << strPassmarkAngle
                                         << strShowSecondPassmark
                                         << strTypeLine
                                         << strPenStyle;

    QString newName = "";
    qint32 index = 0;

    for (int i = 0; i < nodenames.size(); i++)
    {
    QDomNodeList list = elementsByTagName(nodenames[i]);

        for (int j = 0; j < list.size(); j++)
        {
            QDomNode node = list.at(j);

            QDomNamedNodeMap map = node.attributes();

            for (int k = 0; k < map.size(); k++)
            {
                index = -1;
                QDomNode mapItem = map.item(k);
                QDomAttr attribute = mapItem.toAttr();
                QString attributeName = attribute.name();
                QString newName;

                index = oldnames.indexOf(attributeName);
                if (index > -1)
                {
                    QString attributeValue = attribute.value();
                    QDomElement owner = attribute.ownerElement();

                    switch(index)
                    {
                        case 0:
                        {
                            newName = strNotch;
                            owner.setAttribute(strNotchAngle, 0.0);
                            owner.setAttribute(strShowNotch, strTrue);
                            owner.setAttribute(strShowSecond, strTrue);
                            break;
                        }
                        case 1: //! Replaces " passmarkLine" with "notchType"
                        {
                            newName = strNotchType;
                            if (attributeValue == strOne)
                            {
                              attributeValue = strSlitNotch;
                              owner.setAttribute(strNotchCount, 1);
                              break;
                            }
                            if (attributeValue == strTwo)
                            {
                              attributeValue = strSlitNotch;
                              owner.setAttribute(strNotchCount, 2);
                              break;
                            }
                            if (attributeValue == strThree)
                            {
                              attributeValue = strSlitNotch;
                              owner.setAttribute(strNotchCount, 3);
                              break;
                            }
                            if (attributeValue == strTMark)
                            {
                              attributeValue = strT_Notch;
                              owner.setAttribute(strNotchCount, 1);
                              break;
                            }
                            if (attributeValue == strVMark)
                            {
                              attributeValue = strV_External;
                              owner.setAttribute(strNotchCount, 1);
                            }
                            break;
                        }
                        case 2: //! Replaces " passmarkAngle" with "notchSubType"
                        {
                            newName = strNotchSubType;
                            break;
                        }
                        case 3:
                        {
                            newName = strShowSecond;
                            break;
                        }
                        case 4: //! Fixes incorrect name & value for lineType attribute
                        {
                            newName = strLineType;
                            if (attributeValue == strHair)
                            {
                              attributeValue = strSolidLine;
                            }
                            break;
                        }
                        case 5: //! Fixes incorrect value for penStyle attribute
                        {
                            newName = strPenStyle;
                            if (attributeValue == strHair)
                            {
                              attributeValue = strSolidLine;
                            }
                            break;
                        }

                        default:
                            break;
                    }
                    owner.removeAttribute(attributeName);
                    owner.setAttribute(newName, attributeValue);
                }
            }
        }
    }
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_2()
{
    // TODO. Delete if minimal supported version is 0.6.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 2),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.2"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_3()
{
    // TODO. Delete if minimal supported version is 0.6.3
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 3),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_4()
{
    // TODO. Delete if minimal supported version is 0.6.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 4),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.4"));

    // Update tool type attribute
    const QDomNodeList list = elementsByTagName(strPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (!element.isNull())
        {
            const QString type = element.attribute(strType);
            if (type == QStringLiteral("pointOfIntersection"))
            {
                element.removeAttribute(QStringLiteral("pointOfIntersection"));
                element.setAttribute(strType, QStringLiteral("intersectXY"));
                element.setAttribute(strLineType, QStringLiteral("dashLine"));
                element.setAttribute(strLineWeight, QStringLiteral("0.35"));
                element.setAttribute(strLineColor, QStringLiteral("black"));
            }
        }
    }
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_5()
{
    // TODO. Delete if minimal supported version is 0.6.5
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 5),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.5"));

    // Update tool type attribute
    const QDomNodeList list = elementsByTagName(strPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (!element.isNull())
        {
            const QString type = element.attribute(strType);
            if (type == strPin)
            {
                element.removeAttribute(strPin);
                element.setAttribute(strType, strAnchor);
            }
        }
    }

    const QDomNodeList pinList = elementsByTagName(strPins);
    for (int i=0; i < pinList.size(); ++i)
    {
        QDomElement element = pinList.at(i).toElement();
        if (!element.isNull())
        {
            element.setTagName(strAnchors);
        }
    }

    const QDomNodeList dataList = elementsByTagName(strData);
    for (int i=0; i < dataList.size(); ++i)
    {
        QDomElement element = dataList.at(i).toElement();
        if (!element.isNull())
        {
            QDomAttr attribute = element.attributeNode(strCenterPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strCenterPin, strCenterAnchor, attribute.value());
            }

            attribute = element.attributeNode(strTopLeftPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strTopLeftPin, strTopLeftAnchor, attribute.value());
            }

            attribute = element.attributeNode(strBottomRightPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strBottomRightPin, strBottomRightAnchor, attribute.value());
            }
        }
    }

    const QDomNodeList infoList = elementsByTagName(strPatternInfo);
    for (int i=0; i < infoList.size(); ++i)
    {
        QDomElement element = infoList.at(i).toElement();
        if (!element.isNull())
        {
            QDomAttr attribute = element.attributeNode(strCenterPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strCenterPin, strCenterAnchor, attribute.value());
            }

            attribute = element.attributeNode(strTopLeftPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strTopLeftPin, strTopLeftAnchor, attribute.value());
            }

            attribute = element.attributeNode(strBottomRightPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strBottomRightPin, strBottomRightAnchor, attribute.value());
            }
        }
    }

    const QDomNodeList grainlineList = elementsByTagName(strGrainline);
    for (int i=0; i < grainlineList.size(); ++i)
    {
        QDomElement element = grainlineList.at(i).toElement();
        if (!element.isNull())
        {
            QDomAttr attribute = element.attributeNode(strCenterPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strCenterPin, strCenterAnchor, attribute.value());
            }

            attribute = element.attributeNode(strTopPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strTopPin, strTopAnchor, attribute.value());
            }

            attribute = element.attributeNode(strBottomPin);
            if (!attribute.isNull())
            {
                renameAttribute(element, strBottomPin, strBottomAnchor, attribute.value());
            }
        }
    }
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_6()
{
    // TODO. Delete if minimal supported version is 0.6.6
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 6),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.6"));

    // Convert draw to draftBlock
    const QDomNodeList list = elementsByTagName(strDraw);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (!element.isNull())
        {
            element.setTagName(strDraftBlock);
        }
    }

    // Convert details to pieces
    const QDomNodeList detailsList = elementsByTagName(strDetails);
    for (int i=0; i < detailsList.size(); ++i)
    {
        QDomElement element = detailsList.at(i).toElement();
        if (!element.isNull())
        {
            element.setTagName(strPieces);
        }
    }

    // Convert detail to piece
    const QDomNodeList detailList = elementsByTagName(strDetail);
    for (int i=0; i < detailList.size(); ++i)
    {
        QDomElement element = detailList.at(i).toElement();
        if (!element.isNull())
        {
            element.setTagName(strPiece);
        }
    }

    // Convert unionDetail to union
    const QDomNodeList unionList = elementsByTagName(strTools);
    for (int i=0; i < unionList.size(); ++i)
    {
        QDomElement element = unionList.at(i).toElement();
        if (!element.isNull())
        {
            QDomAttr attribute = element.attributeNode(strType);
            if (!attribute.isNull())
            {
                attribute.setValue(strUnion);
            }
        }
    }

    // Convert det to unionPiece
    const QDomNodeList detList = elementsByTagName(QStringLiteral("det"));
    for (int i=0; i < detList.size(); ++i)
    {
        QDomElement element = detList.at(i).toElement();
        if (!element.isNull())
        {
            element.setTagName(QStringLiteral("unionPiece"));
        }
    }
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_7()
{
    // TODO. Delete if minimal supported version is 0.6.7
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 7),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.7"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::toVersion0_6_8()
{
    // TODO. Delete if minimal supported version is 0.6.8
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 8),
                      "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.8"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagUnitToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QDomElement unit = createElement(strUnit);
    QDomText newNodeText = createTextNode(MUnitV0_1_4());
    unit.appendChild(newNodeText);

    QDomElement patternElement = documentElement();
    patternElement.insertAfter(unit, patternElement.firstChildElement(strVersion));
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagIncrementToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    const QSet<QString> names = FixIncrementsToV0_2_0();

    FixPointExpressionsToV0_2_0(names);
    FixArcExpressionsToV0_2_0(names);
    FixPathPointExpressionsToV0_2_0(names);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertMeasurementsToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_2_0();
    ConvertPointExpressionsToV0_2_0(names);
    ConvertArcExpressionsToV0_2_0(names);
    ConvertPathPointExpressionsToV0_2_0(names);
}

//---------------------------------------------------------------------------------------------------------------------
QSet<QString> VPatternConverter::FixIncrementsToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QSet<QString> names;
    const QDomElement incr = TagIncrementsV0_1_4();
    QDomNode domNode = incr.firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                if (domElement.tagName() == strIncrement)
                {
                    try
                    {
                        const QString name = GetParametrString(domElement, strName);
                        names.insert(name);
                        domElement.setAttribute(strName, QLatin1String("#")+name);

                        const QString base = GetParametrString(domElement, strBase);
                        domElement.setAttribute(strFormula, base);
                    }
                    catch (VExceptionEmptyParameter &error)
                    {
                        VException excep("Can't get increment.");
                        excep.AddMoreInformation(error.ErrorMessage());
                        throw excep;
                    }
                    domElement.removeAttribute(strId);
                    domElement.removeAttribute(strKGrowth);
                    domElement.removeAttribute(strKSize);
                    domElement.removeAttribute(strBase);
                }
            }
        }
        domNode = domNode.nextSibling();
    }
    return names;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixPointExpressionsToV0_2_0(const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(strPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, strLength);
            dom.setAttribute(strLength, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strAngle);
            dom.setAttribute(strAngle, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
        try
        {
            formula = GetParametrString(dom, strC1Radius);
            dom.setAttribute(strC1Radius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strC2Radius);
            dom.setAttribute(strC2Radius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strCRadius);
            dom.setAttribute(strCRadius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixArcExpressionsToV0_2_0(const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(strArc);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, strAngle1);
            dom.setAttribute(strAngle1, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strAngle2);
            dom.setAttribute(strAngle2, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strRadius);
            dom.setAttribute(strRadius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strLength);
            dom.setAttribute(strLength, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixPathPointExpressionsToV0_2_0(const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(strPathPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, strKAsm1);
            dom.setAttribute(strKAsm1, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strKAsm2);
            dom.setAttribute(strKAsm2, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strAngle);
            dom.setAttribute(strAngle, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertPointExpressionsToV0_2_0(const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(strPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, strLength);
            dom.setAttribute(strLength, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strAngle);
            dom.setAttribute(strAngle, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
        try
        {
            formula = GetParametrString(dom, strC1Radius);
            dom.setAttribute(strC1Radius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strC2Radius);
            dom.setAttribute(strC2Radius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strCRadius);
            dom.setAttribute(strCRadius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertArcExpressionsToV0_2_0(const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(strArc);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, strAngle1);
            dom.setAttribute(strAngle1, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strAngle2);
            dom.setAttribute(strAngle2, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strRadius);
            dom.setAttribute(strRadius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strLength);
            dom.setAttribute(strLength, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertPathPointExpressionsToV0_2_0(const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(strPathPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, strKAsm1);
            dom.setAttribute(strKAsm1, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strKAsm2);
            dom.setAttribute(strKAsm2, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        try
        {
            formula = GetParametrString(dom, strAngle);
            dom.setAttribute(strAngle, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VPatternConverter::FixMeasurementInFormulaToV0_2_0(const QString &formula, const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, false, false));// Eval formula
    QMap<int, QString> tokens = cal->GetTokens();// Tokens (variables, measurements)
    delete cal.take();

    QList<int> tKeys = tokens.keys();// Take all tokens positions
    QList<QString> tValues = tokens.values();

    QString newFormula = formula;// Local copy for making changes
    for (int i = 0; i < tValues.size(); ++i)
    {
        if (not names.contains(tValues.at(i)))
        {
            continue;
        }

        int bias = 0;
        Replace(newFormula, names.value(tValues.at(i)), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        {// Translated token has different length than original. Position next tokens need to be corrected.
            CorrectionsPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPatternConverter::FixIncrementInFormulaToV0_2_0(const QString &formula, const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    qmu::QmuTokenParser *cal = new qmu::QmuTokenParser(formula, false, false);// Eval formula
    QMap<int, QString> tokens = cal->GetTokens();// Tokens (variables, measurements)
    delete cal;

    QList<int> tKeys = tokens.keys();// Take all tokens positions
    QList<QString> tValues = tokens.values();

    QString newFormula = formula;// Local copy for making changes
    for (int i = 0; i < tValues.size(); ++i)
    {
        if (not names.contains(tValues.at(i)))
        {
            continue;
        }

        int bias = 0;
        Replace(newFormula, "#"+tValues.at(i), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        {// Translated token has different length than original. Position next tokens need to be corrected.
            CorrectionsPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagMeasurementsToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    QDomElement ms = TagMeasurementsV0_1_4();
    const QString path = GetParametrString(ms, strPath);

    ms.removeAttribute(strUnit);
    ms.removeAttribute(strType);
    ms.removeAttribute(strPath);

    QDomText newNodeText = createTextNode(QFileInfo(m_convertedFileName).absoluteDir().relativeFilePath(path));
    ms.appendChild(newNodeText);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertMeasurementsToV0_2_1()
{
    // TODO. Delete if minimal supported version is 0.2.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 1),
                      "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_2_1();

    // Structure did not change. We can use the same code.
    ConvertPointExpressionsToV0_2_0(names);
    ConvertArcExpressionsToV0_2_0(names);
    ConvertPathPointExpressionsToV0_2_0(names);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveColorToolCutV0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 1),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(strType);
            if (type == strCutArc || type == strCutSpline || type == strCutSplinePath)
            {
                element.removeAttribute(strColor);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VPatternConverter::MUnitV0_1_4() const
{
    // TODO. Delete if minimal supported version is 0.1.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 4),
                      "Time to refactor the code.");

    const QDomElement element = TagMeasurementsV0_1_4();
    try
    {
        return GetParametrString(element, strUnit);
    }
    catch (VExceptionEmptyParameter &error)
    {
        VException excep("Can't get unit.");
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPatternConverter::TagMeasurementsV0_1_4() const
{
    // TODO. Delete if minimal supported version is 0.1.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 4),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strMeasurements);
    const QDomElement element = list.at(0).toElement();
    if (not element.isElement())
    {
        VException excep("Can't get tag measurements.");
        throw excep;
    }
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPatternConverter::TagIncrementsV0_1_4() const
{
    // TODO. Delete if minimal supported version is 0.1.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 1, 4),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strIncrements);
    const QDomElement element = list.at(0).toElement();
    if (not element.isElement())
    {
        VException excep("Can't get tag measurements.");
        throw excep;
    }
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixToolUnionToV0_2_4()
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 4),
                      "Time to refactor the code.");

    QDomElement root = documentElement();
    const QDomNodeList modelings = root.elementsByTagName(strModeling);
    for (int i=0; i<modelings.size(); ++i)
    {
        ParseModelingToV0_2_4(modelings.at(i).toElement());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ParseModelingToV0_2_4(const QDomElement &modeling)
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 4),
                      "Time to refactor the code.");

    QDomElement node = modeling.firstChild().toElement();
    while (not node.isNull())
    {
        if (node.tagName() == strTools)
        {
            const quint32 toolId = node.attribute(strId).toUInt();
            QVector<quint32> children;
            QDomElement childNode = node.nextSibling().toElement();
            while (not childNode.isNull())
            {
                if (childNode.hasAttribute(strIdTool) && childNode.attribute(strIdTool).toUInt() == toolId)
                {
                    children.append(childNode.attribute(strIdObject).toUInt());
                }
                else
                {
                    break;
                }
                childNode = childNode.nextSibling().toElement();
            }

            if (not children.isEmpty())
            {
                SaveChildrenToolUnionToV0_2_4(toolId, children);
            }
            node = childNode;
            continue;
        }
        node = node.nextSibling().toElement();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::SaveChildrenToolUnionToV0_2_4(quint32 id, const QVector<quint32> &children)
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 4),
                      "Time to refactor the code.");

    QDomElement toolUnion = elementById(id);
    if (toolUnion.isNull())
    {
        return;
    }

    QDomElement tagChildren = createElement(strChildren);

    for (int i=0; i<children.size(); ++i)
    {
        QDomElement tagChild = createElement(strChild);
        tagChild.appendChild(createTextNode(QString().setNum(children.at(i))));
        tagChildren.appendChild(tagChild);
    }

    toolUnion.appendChild(tagChildren);
}

//---------------------------------------------------------------------------------------------------------------------
QMap<QString, QString> VPatternConverter::OldNamesToNewNames_InV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 0),
                      "Time to refactor the code.");

    // old name, new name
    QMap<QString, QString> names;

    names.insert(QStringLiteral("cervicale_height"), QStringLiteral("height_neck_back"));
    names.insert(QStringLiteral("height_scapular_point"), QStringLiteral("height_scapula"));
    names.insert(QStringLiteral("height_back_angle_axilla"), QStringLiteral("height_armpit"));
    names.insert(QStringLiteral("waist_height"), QStringLiteral("height_waist_side"));
    names.insert(QStringLiteral("hip_height"), QStringLiteral("height_hip"));
    names.insert(QStringLiteral("knee_height"), QStringLiteral("height_knee"));
    names.insert(QStringLiteral("ankle_height"), QStringLiteral("height_ankle"));
    names.insert(QStringLiteral("high_hip_height"), QStringLiteral("height_highhip"));
    names.insert(QStringLiteral("front_waist_to_floor"), QStringLiteral("height_waist_front"));
    names.insert(QStringLiteral("height_nipple_point"), QStringLiteral("height_bustpoint"));

    QString name = QStringLiteral("height_shoulder_tip");
    names.insert(QStringLiteral("shoulder_height"), name);
    names.insert(QStringLiteral("height_shoulder_point"), name);

    name = QStringLiteral("height_neck_front");
    names.insert(QStringLiteral("height_clavicular_point"), name);
    names.insert(QStringLiteral("height_front_neck_base_point"), name);

    names.insert(QStringLiteral("height_base_neck_side_point"), QStringLiteral("height_neck_side"));

    name = QStringLiteral("height_neck_back_to_knee");
    names.insert(QStringLiteral("neck_to_knee_point"), name);
    names.insert(QStringLiteral("cervicale_to_knee_height"), name);

    names.insert(QStringLiteral("waist_to_knee_height"), QStringLiteral("height_waist_side_to_knee"));
    names.insert(QStringLiteral("waist_to_hip_height"), QStringLiteral("height_waist_side_to_hip"));
    names.insert(QStringLiteral("body_position"), QStringLiteral("indent_neck_back"));

    name = QStringLiteral("neck_mid_circ");
    names.insert(QStringLiteral("half_girth_neck_for_shirts"), name);
    names.insert(QStringLiteral("mid_neck_girth"), name);

    names.insert(QStringLiteral("neck_base_girth"), QStringLiteral("neck_circ"));
    names.insert(QStringLiteral("upper_chest_girth"), QStringLiteral("highbust_circ"));
    names.insert(QStringLiteral("bust_girth"), QStringLiteral("bust_circ"));
    names.insert(QStringLiteral("under_bust_girth"), QStringLiteral("lowbust_circ"));
    names.insert(QStringLiteral("waist_girth"), QStringLiteral("waist_circ"));
    names.insert(QStringLiteral("high_hip_girth"), QStringLiteral("highhip_circ"));
    names.insert(QStringLiteral("hips_excluding_protruding_abdomen"), QStringLiteral("hip_circ"));
    names.insert(QStringLiteral("hip_girth"), QStringLiteral("hip_circ_with_abdomen"));

    name = QStringLiteral("neck_arc_f");
    names.insert(QStringLiteral("half_girth_neck"), name);
    names.insert(QStringLiteral("front_neck_arc"), name);

    name = QStringLiteral("highbust_arc_f");
    names.insert(QStringLiteral("half_girth_chest_first"), name);
    names.insert(QStringLiteral("front_upper_chest_arc"), name);

    names.insert(QStringLiteral("half_girth_chest_second"), QStringLiteral("bust_arc_f"));
    names.insert(QStringLiteral("half_girth_chest_third"), QStringLiteral("lowbust_arc_f"));

    name = QStringLiteral("waist_arc_f");
    names.insert(QStringLiteral("half_girth_waist"), name);
    names.insert(QStringLiteral("front_waist_arc"), name);

    names.insert(QStringLiteral("front_upper_hip_arc"), QStringLiteral("highhip_arc_f"));

    name = QStringLiteral("hip_arc_f");
    names.insert(QStringLiteral("half_girth_hips_excluding_protruding_abdomen"), name);
    names.insert(QStringLiteral("front_hip_arc"), name);

    names.insert(QStringLiteral("back_neck_arc"), QStringLiteral("neck_arc_b"));
    names.insert(QStringLiteral("back_upper_chest_arc"), QStringLiteral("highbust_arc_b"));
    names.insert(QStringLiteral("back_waist_arc"), QStringLiteral("waist_arc_b"));
    names.insert(QStringLiteral("back_upper_hip_arc"), QStringLiteral("highhip_arc_b"));
    names.insert(QStringLiteral("back_hip_arc"), QStringLiteral("hip_arc_b"));
    names.insert(QStringLiteral("half_girth_hips_considering_protruding_abdomen"),
                 QStringLiteral("hip_with_abdomen_arc_f"));
    names.insert(QStringLiteral("shoulder_girth"), QStringLiteral("body_armfold_circ"));
    names.insert(QStringLiteral("trunk_length"), QStringLiteral("body_torso_circ"));
    names.insert(QStringLiteral("front_waist_length"), QStringLiteral("neck_front_to_waist_f"));
    names.insert(QStringLiteral("center_front_waist_length"), QStringLiteral("neck_front_to_waist_flat_f"));
    names.insert(QStringLiteral("side_waist_length"), QStringLiteral("armpit_to_waist_side"));
    names.insert(QStringLiteral("waist_to_neck_side"), QStringLiteral("neck_side_to_waist_b"));

    name = QStringLiteral("neck_side_to_waist_f");
    names.insert(QStringLiteral("neck_to_front_waist_line"), name);
    names.insert(QStringLiteral("front_shoulder_to_waist_length"), name);

    names.insert(QStringLiteral("back_shoulder_to_waist_length"), QStringLiteral("neck_side_to_waist_b"));
    names.insert(QStringLiteral("center_back_waist_length"), QStringLiteral("neck_back_to_waist_b"));

    name = QStringLiteral("neck_front_to_highbust_f");
    names.insert(QStringLiteral("neck_to_first_line_chest_circumference"), name);
    names.insert(QStringLiteral("front_neck_to_upper_chest_height"), name);

    names.insert(QStringLiteral("front_neck_to_bust_height"), QStringLiteral("neck_front_to_bust_f"));
    names.insert(QStringLiteral("front_waist_to_upper_chest"), QStringLiteral("highbust_to_waist_f"));
    names.insert(QStringLiteral("front_waist_to_lower_breast"), QStringLiteral("lowbust_to_waist_f"));
    names.insert(QStringLiteral("neck_to_back_line_chest_circumference"), QStringLiteral("neck_back_to_highbust_b"));
    names.insert(QStringLiteral("depth_waist_first"), QStringLiteral("waist_to_highhip_f"));
    names.insert(QStringLiteral("depth_waist_second"), QStringLiteral("waist_to_hip_f"));
    names.insert(QStringLiteral("shoulder_slope_degrees"), QStringLiteral("shoulder_slope_neck_side_angle"));
    names.insert(QStringLiteral("shoulder_drop"), QStringLiteral("shoulder_slope_neck_side_length"));
    names.insert(QStringLiteral("across_front_shoulder_width"), QStringLiteral("shoulder_tip_to_shoulder_tip_f"));
    names.insert(QStringLiteral("upper_front_chest_width"), QStringLiteral("across_chest_f"));
    names.insert(QStringLiteral("chest_width"), QStringLiteral("across_chest_f"));
    names.insert(QStringLiteral("front_chest_width"), QStringLiteral("armfold_to_armfold_f"));

    name = QStringLiteral("shoulder_tip_to_shoulder_tip_b");
    names.insert(QStringLiteral("arc_behind_shoulder_girdle"), name);
    names.insert(QStringLiteral("across_back_shoulder_width"), name);

    names.insert(QStringLiteral("upper_back_width"), QStringLiteral("across_back_b"));
    names.insert(QStringLiteral("back_width"), QStringLiteral("armfold_to_armfold_b"));
    names.insert(QStringLiteral("neck_transverse_diameter"), QStringLiteral("neck_width"));
    names.insert(QStringLiteral("bustpoint_to_bustpoint"), QStringLiteral("bustpoint_to_bustpoint"));
    names.insert(QStringLiteral("neck_to_bustpoint"), QStringLiteral("bustpoint_to_neck_side"));
    names.insert(QStringLiteral("halter_bustpoint_to_bustpoint"), QStringLiteral("bustpoint_to_bustpoint_halter"));

    name = QStringLiteral("shoulder_tip_to_waist_front");
    names.insert(QStringLiteral("front_slash_shoulder_height"), name);
    names.insert(QStringLiteral("front_shoulder_slope_length"), name);

    names.insert(QStringLiteral("front_waist_slope"), QStringLiteral("neck_front_to_waist_side"));

    name = QStringLiteral("neck_side_to_armfold_f");
    names.insert(QStringLiteral("height_armhole_slash"), name);
    names.insert(QStringLiteral("chest_slope"), name);

    name = QStringLiteral("shoulder_tip_to_waist_back");
    names.insert(QStringLiteral("slash_shoulder_height"), name);
    names.insert(QStringLiteral("back_shoulder_slope_length"), name);

    names.insert(QStringLiteral("back_waist_slope"), QStringLiteral("neck_back_to_waist_side"));
    names.insert(QStringLiteral("back_slope"), QStringLiteral("neck_side_to_armfold_b"));
    names.insert(QStringLiteral("arm_length"), QStringLiteral("arm_shoulder_tip_to_wrist"));
    names.insert(QStringLiteral("shoulder_to_elbow_length"), QStringLiteral("arm_shoulder_tip_to_elbow"));
    names.insert(QStringLiteral("underarm_length"), QStringLiteral("arm_armpit_to_wrist"));
    names.insert(QStringLiteral("upper_arm_girth"), QStringLiteral("arm_upper_circ"));
    names.insert(QStringLiteral("wrist_girth"), QStringLiteral("arm_wrist_circ"));
    names.insert(QStringLiteral("armscye_girth"), QStringLiteral("armscye_circ"));
    names.insert(QStringLiteral("anteroposterior_diameter_hands"), QStringLiteral("armscye_width"));
    names.insert(QStringLiteral("neck_to_third_finger"), QStringLiteral("arm_neck_side_to_finger_tip"));
    names.insert(QStringLiteral("neck_to_radial_point"), QStringLiteral("arm_neck_side_to_outer_elbow"));
    names.insert(QStringLiteral("shoulder_and_arm_length"), QStringLiteral("arm_neck_side_to_wrist"));
    names.insert(QStringLiteral("crotch_height"), QStringLiteral("leg_crotch_to_floor"));
    names.insert(QStringLiteral("side_waist_to_floor"), QStringLiteral("leg_waist_side_to_floor"));
    names.insert(QStringLiteral("waist_to_knee"), QStringLiteral("leg_waist_side_to_knee"));
    names.insert(QStringLiteral("thigh_girth"), QStringLiteral("leg_thigh_upper_circ"));
    names.insert(QStringLiteral("mid_thigh_girth"), QStringLiteral("leg_thigh_mid_circ"));
    names.insert(QStringLiteral("knee_girth"), QStringLiteral("leg_knee_circ"));
    names.insert(QStringLiteral("calf_girth"), QStringLiteral("leg_calf_circ"));
    names.insert(QStringLiteral("ankle_girth"), QStringLiteral("leg_ankle_circ"));
    names.insert(QStringLiteral("girth_knee_flexed_feet"), QStringLiteral("leg_knee_circ_bent"));
    names.insert(QStringLiteral("arc_through_groin_area"), QStringLiteral("crotch_length"));
    names.insert(QStringLiteral("waist_to_plane_seat"), QStringLiteral("rise_length_side_sitting"));
    names.insert(QStringLiteral("rise_height"), QStringLiteral("rise_length_diag"));
    names.insert(QStringLiteral("hand_vertical_diameter"), QStringLiteral("hand_length"));
    names.insert(QStringLiteral("hand_width"), QStringLiteral("hand_palm_width"));
    names.insert(QStringLiteral("hand_girth"), QStringLiteral("hand_circ"));
    names.insert(QStringLiteral("girth_foot_instep"), QStringLiteral("foot_instep_circ"));
    names.insert(QStringLiteral("head_height"), QStringLiteral("head_length"));
    names.insert(QStringLiteral("head_and_neck_length"), QStringLiteral("head_crown_to_neck_back"));
    names.insert(QStringLiteral("neck_to_neck_base"), QStringLiteral("head_chin_to_neck_back"));
    names.insert(QStringLiteral("arc_length_upper_body"), QStringLiteral("waist_to_waist_halter"));
    names.insert(QStringLiteral("cervicale_to_wrist_length"), QStringLiteral("arm_neck_back_to_wrist_bent"));
    names.insert(QStringLiteral("strap_length"), QStringLiteral("highbust_b_over_shoulder_to_highbust_f"));
    names.insert(QStringLiteral("arc_through_shoulder_joint"), QStringLiteral("armscye_arc"));
    names.insert(QStringLiteral("head_girth"), QStringLiteral("head_circ"));
    names.insert(QStringLiteral("elbow_girth"), QStringLiteral("arm_elbow_circ"));
    names.insert(QStringLiteral("height_under_buttock_folds"), QStringLiteral("height_gluteal_fold"));
    names.insert(QStringLiteral("scye_depth"), QStringLiteral("neck_back_to_highbust_b"));
    names.insert(QStringLiteral("back_waist_to_upper_chest"), QStringLiteral("across_back_to_waist_b"));

    return names;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<QString, QString> VPatternConverter::OldNamesToNewNames_InV0_2_1()
{
    // TODO. Delete if minimal supported version is 0.2.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 2, 1),
                      "Time to refactor the code.");

    // old name, new name
    QMap<QString, QString> names;

    names.insert(QStringLiteral("rise_length_side"), QStringLiteral("rise_length_side_sitting"));
    names.insert(QStringLiteral("size"), QStringLiteral("bust_arc_f"));

    return names;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixCutPoint()
{
    const QStringList types = QStringList() << strCutSplinePath
                                            << strCutSpline
                                            << strCutArc;

    const QDomNodeList list = elementsByTagName(strPoint);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(strType);
            switch(types.indexOf(type))
            {
                case 0: //strCutSplinePath
                {
                    const quint32 id = element.attribute(strId).toUInt();
                    quint32 curve = element.attribute(strSplinePath).toUInt();
                    FixSubPaths(i, id, curve);
                    break;
                }
                case 1: //strCutSpline
                {
                    const quint32 id = element.attribute(strId).toUInt();
                    quint32 curve = element.attribute(strSpline).toUInt();
                    FixSubPaths(i, id, curve);
                    break;
                }
                case 2: //strCutArc
                {
                    const quint32 id = element.attribute(strId).toUInt();
                    quint32 curve = element.attribute(strArc).toUInt();
                    FixSubPaths(i, id, curve);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixSubPaths(int i, quint32 id, quint32 baseCurve)
{
    const QStringList pointTypes = QStringList() << strCutSplinePath
                                                 << strCutSpline
                                                 << strPointOfIntersectionCurves
                                                 << strCurveIntersectAxis
                                                 << strPointFromArcAndTangent
                                                 << strPointOfIntersectionArcs;

    const QDomNodeList listPoints = elementsByTagName(strPoint);
    for (int j = i+1; j < listPoints.size(); ++j)
    {
        QDomElement element = listPoints.at(j).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(strType);
            switch(pointTypes.indexOf(type))
            {
                case 0: //strCutSplinePath
                {
                    const quint32 spl = element.attribute(strSplinePath).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strSplinePath, baseCurve);
                    }
                    break;
                }
                case 1: //strCutSpline
                {
                    const quint32 spl = element.attribute(strSpline).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strSpline, baseCurve);
                    }
                    break;
                }
                case 2: //strPointOfIntersectionCurves
                {
                    quint32 spl = element.attribute(strCurve1).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strCurve1, baseCurve);
                    }

                    spl = element.attribute(strCurve2).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strCurve2, baseCurve);
                    }
                    break;
                }
                case 3: //strCurveIntersectAxis
                {
                    const quint32 spl = element.attribute(strCurve).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strCurve, baseCurve);
                    }
                    break;
                }
                case 4: //strPointFromArcAndTangent
                {
                    const quint32 spl = element.attribute(strArc).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strArc, baseCurve);
                    }
                    break;
                }
                case 5: //strPointOfIntersectionArcs
                {
                    quint32 arc = element.attribute(strFirstArc).toUInt();
                    if (arc == id+1 || arc == id+2)
                    {
                        element.setAttribute(strFirstArc, baseCurve);
                    }

                    arc = element.attribute(strSecondArc).toUInt();
                    if (arc == id+1 || arc == id+2)
                    {
                        element.setAttribute(strSecondArc, baseCurve);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    const QStringList splTypes = QStringList() << strModelingPath
                                               << strModelingSpline;

    const QDomNodeList listSplines = elementsByTagName(strSpline);
    for (int j = 0; j < listSplines.size(); ++j)
    {
        QDomElement element = listSplines.at(j).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(strType);
            switch(splTypes.indexOf(type))
            {
                case 0: //strModelingPath
                case 1: //strModelingSpline
                {
                    const quint32 spl = element.attribute(strIdObject).toUInt();
                    if (spl == id+1 || spl == id+2)
                    {
                        element.setAttribute(strIdObject, baseCurve);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    const QDomNodeList listArcs = elementsByTagName(strArc);
    for (int j = 0; j < listArcs.size(); ++j)
    {
        QDomElement element = listArcs.at(j).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(strType);
            if (type == strModeling)
            {
                const quint32 arc = element.attribute(strIdObject).toUInt();
                if (arc == id+1 || arc == id+2)
                {
                    element.setAttribute(strIdObject, baseCurve);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagRemoveAttributeTypeObjectInV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strModeling);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement modeling = list.at(i).toElement();
        if (not modeling.isNull())
        {
            QDomNode domNode = modeling.firstChild();
            while (not domNode.isNull())
            {
                QDomElement domElement = domNode.toElement();
                if (not domElement.isNull())
                {
                    if (domElement.hasAttribute(strTypeObject))
                    {
                        domElement.removeAttribute(strTypeObject);
                    }
                }
                domNode = domNode.nextSibling();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagDetailToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strDetail);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        if (not dom.isNull())
        {
            dom.setAttribute(strSeamAllowance, dom.attribute(strSupplement, "0"));
            dom.removeAttribute(strSupplement);

            dom.setAttribute(strVersion, "1");

            const QStringList tags = QStringList() << strNode << strData << strPatternInfo << strGrainline;

            QDomElement tagData;
            QDomElement tagPatternInfo;
            QDomElement tagGrainline;
            QDomElement tagNodes = createElement(strNodes);

            const QDomNodeList childList = dom.childNodes();
            for (qint32 i = 0; i < childList.size(); ++i)
            {
                const QDomElement element = childList.at(i).toElement();
                if (not element.isNull())
                {
                    switch (tags.indexOf(element.tagName()))
                    {
                        case 0://strNode
                        {
                            QDomElement tagNode = createElement(strNode);

                            tagNode.setAttribute(strIdObject, element.attribute(strIdObject, NULL_ID_STR));

                            if (element.hasAttribute(strReverse))
                            {
                                tagNode.setAttribute(strReverse, element.attribute(strReverse, "0"));
                            }

                            if (element.hasAttribute(strMx))
                            {
                                tagNode.setAttribute(strMx, element.attribute(strMx, "0"));
                            }

                            if (element.hasAttribute(strMy))
                            {
                                tagNode.setAttribute(strMy, element.attribute(strMy, "0"));
                            }

                            tagNode.setAttribute(strType, element.attribute(strType, ""));

                            tagNodes.appendChild(tagNode);

                            break;
                        }
                        case 1://strData
                            tagData = element.cloneNode().toElement();
                            break;
                        case 2://strPatternInfo
                            tagPatternInfo = element.cloneNode().toElement();
                            break;
                        case 3://strGrainline
                            tagGrainline = element.cloneNode().toElement();
                            break;
                        default:
                            break;
                    }
                }
            }

            RemoveAllChildren(dom);

            dom.appendChild(tagData);
            dom.appendChild(tagPatternInfo);
            dom.appendChild(tagGrainline);
            dom.appendChild(tagNodes);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPatternConverter::GetUnionDetailNodesV0_4_0(const QDomElement &piece)
{
    QDomElement tagNodes = createElement(strNodes);

    if (not piece.isNull())
    {
        const QDomNodeList childList = piece.childNodes();
        for (qint32 i = 0; i < childList.size(); ++i)
        {
            const QDomElement node = childList.at(i).toElement();
            if (not node.isNull())
            {
                QDomElement tagNode = createElement(strNode);

                tagNode.setAttribute(strIdObject, node.attribute(strIdObject, NULL_ID_STR));

                if (node.hasAttribute(strReverse))
                {
                    tagNode.setAttribute(strReverse, node.attribute(strReverse, "0"));
                }

                tagNode.setAttribute(strType, node.attribute(strType, ""));

                tagNodes.appendChild(tagNode);
            }
        }
    }

    return tagNodes;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPatternConverter::GetUnionChildrenNodesV0_4_0(const QDomElement &piece)
{
    QDomElement tagNodes = createElement(strNodes);

    if (not piece.isNull())
    {
        const QDomNodeList childList = piece.childNodes();
        for (qint32 i = 0; i < childList.size(); ++i)
        {
            const QDomElement node = childList.at(i).toElement();
            if (not node.isNull())
            {
                QDomElement tagNode = node.cloneNode().toElement();
                tagNodes.appendChild(tagNode);
            }
        }
    }

    return tagNodes;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::LabelTagToV0_4_4(const QString &tagName)
{
    // TODO. Delete if minimal supported version is 0.4.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 4),
                      "Time to refactor the code.");

    Unit unit = Unit::Cm;
    const QStringList units = QStringList() << "mm" << "cm" << "inch";
    switch (units.indexOf(UniqueTagText(strUnit)))
    {
        case 0:// mm
            unit = Unit::Mm;
            break;
        case 1:// cm
            unit = Unit::Cm;
            break;
        case 2:// in
            unit = Unit::Inch;
            break;
        default:
            break;
    }

    auto ConvertData = [unit](QDomElement &dom, const QString &attribute)
    {
        if (dom.hasAttribute(attribute))
        {
            QString valStr = dom.attribute(attribute, "1");
            bool ok = false;
            qreal val = valStr.toDouble(&ok);
            if (not ok)
            {
                val = 1;
            }
            dom.setAttribute(attribute, QString().setNum(FromPixel(val, unit)));
        }
    };

    const QDomNodeList list = elementsByTagName(tagName);
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        if (not dom.isNull())
        {
            ConvertData(dom, strWidth);
            ConvertData(dom, strHeight);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPatternConverter::AddTagPatternLabelV0_5_1()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strPatternLabel);
    if (list.isEmpty())
    {
        const QStringList tags = QStringList() << strUnit
                                               << strImage
                                               << strAuthor
                                               << strDescription
                                               << strNotes
                                               << strGradation
                                               << strPatternName
                                               << strPatternNum
                                               << strCompanyName
                                               << strCustomerName
                                               << strPatternLabel;

        QDomElement element = createElement(strPatternLabel);
        QDomElement pattern = documentElement();
        for (int i = tags.indexOf(element.tagName())-1; i >= 0; --i)
        {
            const QDomNodeList list = elementsByTagName(tags.at(i));
            if (not list.isEmpty())
            {
                pattern.insertAfter(element, list.at(0));
                break;
            }
        }
        return element;
    }
    return list.at(0).toElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::PortPatternLabeltoV0_6_0(QDomElement &label)
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");

    if (not UniqueTagText(strCompanyName).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%author%", true, false, 0, 4);
    }
    else
    {
        const QString author = UniqueTagText(strAuthor);
        if (not author.isEmpty())
        {
            AddLabelTemplateLineV0_6_0(label, author, true, false, 0, 4);
        }
    }

    if (not UniqueTagText(strPatternName).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%patternName%", false, false, 0, 2);
    }

    if (not UniqueTagText(strPatternNum).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%patternNumber%", false, false, 0, 0);
    }

    if (not UniqueTagText(strCustomerName).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%customer%", false, true, 0, 0);
    }

    const QString sizeField = UniqueTagText(strSize);
    if (not sizeField.isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, sizeField, false, false, 0, 0);
    }

    if (UniqueTagText(strShowMeasurements) == trueStr)
    {
        AddLabelTemplateLineV0_6_0(label, "%mFileName%.%mExt%", false, false, 0, 0);
    }

    if (UniqueTagText(strShowDate) == trueStr)
    {
        AddLabelTemplateLineV0_6_0(label, "%date%", false, true, 0, 0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::AddLabelTemplateLineV0_6_0(QDomElement &label, const QString &text, bool bold, bool italic,
                                                   int alignment, int fontSizeIncrement)
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");

    QDomElement tagLine = createElement(strLine);

    SetAttribute(tagLine, strText, text);
    SetAttribute(tagLine, strBold, bold);
    SetAttribute(tagLine, strItalic, italic);
    SetAttribute(tagLine, strAlignment, alignment);
    SetAttribute(tagLine, strFSIncrement, fontSizeIncrement);

    label.appendChild(tagLine);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::PortPieceLabelstoV0_6_0()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");

    const QDomNodeList nodeList = elementsByTagName(strData);
    for (int i = 0; i < nodeList.size(); ++i)
    {
        QDomElement dataTag = nodeList.at(i).toElement();
        QDomNodeList nodeListMCP = dataTag.childNodes();
        const int count = nodeListMCP.count();
        try
        {
            if (not GetParametrString(dataTag, strLetter, "").isEmpty())
            {
                AddLabelTemplateLineV0_6_0(dataTag, "%pLetter%", true, false, Qt::AlignHCenter, 6);
            }
        }
        catch(const VExceptionEmptyParameter &)
        {}

        AddLabelTemplateLineV0_6_0(dataTag, "%pName%", true, false, Qt::AlignHCenter, 2);

        for (int iMCP = 0; iMCP < count; ++iMCP)
        {
            QDomElement domMCP = nodeListMCP.at(iMCP).toElement();

            QString line;

            const int material = static_cast<int>(GetParametrUInt(domMCP, strMaterial, "0"));
            switch(material)
            {
                case 0:
                    line.append("%mFabric%");
                    break;
                case 1:
                    line.append("%mLining%");
                    break;
                case 2:
                    line.append("%mInterfacing%");
                    break;
                case 3:
                    line.append("%mInterlining%");
                    break;
                case 4:
                default:
                    line.append(GetParametrString(domMCP, strUserDefined, "User material"));
                    break;
            }

            line.append(", %wCut% %pQuantity%");
            if (GetParametrUInt(domMCP, strPlacement, "0") == 1)
            {
                line.append(" %wOnFold%");
            }

            AddLabelTemplateLineV0_6_0(dataTag, line, false, false, Qt::AlignHCenter, 0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveUnusedTagsV0_6_0()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");

    RemoveUniqueTagV0_6_0(strAuthor);
    RemoveUniqueTagV0_6_0(strSize);
    RemoveUniqueTagV0_6_0(strShowDate);
    RemoveUniqueTagV0_6_0(strShowMeasurements);

    QDomNodeList nodeList = elementsByTagName(strData);
    for (int i = 0; i < nodeList.size(); ++i)
    {
        QDomElement child = nodeList.at(i).firstChildElement(strMCP);
        while (not child.isNull())
        {
            nodeList.at(i).removeChild(child);
            child = nodeList.at(i).firstChildElement(strMCP);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveUniqueTagV0_6_0(const QString &tag)
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 6, 0),
                      "Time to refactor the code.");

    const QDomNodeList nodeList = elementsByTagName(tag);
    if (nodeList.isEmpty())
    {
        return;
    }

    QDomElement pattern = documentElement();
    pattern.removeChild(nodeList.at(0));
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagUnionDetailsToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(strTools);
    for (int i=0; i < list.size(); ++i)
    {
        // Tag 'tools' used only for union pieces, so no need to check any additional attributes
        QDomElement toolDOM = list.at(i).toElement();
        if (not toolDOM.isNull())
        {
            const QStringList tags = QStringList() << strDet << strChildren;

            QVector<QDomElement> nodes;
            QDomElement tagChildrenNodes = createElement(strChildren);

            const QDomNodeList childList = toolDOM.childNodes();
            for (qint32 i = 0; i < childList.size(); ++i)
            {
                const QDomElement element = childList.at(i).toElement();
                if (not element.isNull())
                {
                    switch (tags.indexOf(element.tagName()))
                    {
                        case 0://strDet
                            nodes.append(GetUnionDetailNodesV0_4_0(element));
                            break;
                        case 1://strChildren
                            tagChildrenNodes.appendChild(GetUnionChildrenNodesV0_4_0(element));
                            break;
                        default:
                            break;
                    }
                }
            }

            RemoveAllChildren(toolDOM);

            for (int i = 0; i < nodes.size(); ++i)
            {
                QDomElement tagDet = createElement(strDet);
                tagDet.appendChild(nodes.at(i));
                toolDOM.appendChild(tagDet);
            }
            toolDOM.appendChild(tagChildrenNodes);
        }
    }
}

void VPatternConverter::renameAttribute(QDomElement &element, const QString &oldName,
                                        const QString &newName, const QString &value) const
{
    element.removeAttribute(oldName);
    element.setAttribute(newName, value);
}
