/******************************************************************************
 *   @file   vtoolellipticalarc.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/
/************************************************************************
 **
 **  @file   vtoolellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   20 10, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "vtoolellipticalarc.h"

#include <QPen>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <Qt>
#include <new>

#include "../../../dialogs/tools/dialogtool.h"
#include "../../../dialogs/tools/dialogellipticalarc.h"
#include "../../../visualization/path/vistoolellipticalarc.h"
#include "../../../visualization/visualization.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"
#include "vabstractspline.h"

const QString VToolEllipticalArc::ToolType = QStringLiteral("simple");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolEllipticalArc constuctor.
 * @param doc dom document container
 * @param data container with variables
 * @param id object id in container
 * @param typeCreation way we create this tool.
 * @param parent parent object
 */
VToolEllipticalArc::VToolEllipticalArc(VAbstractPattern *doc, VContainer *data, quint32 id, const Source &typeCreation,
                                       QGraphicsItem *parent)
    :VAbstractSpline(doc, data, id, parent)
{
    sceneType = SceneObject::ElArc;

    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus

    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolEllipticalArc::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogEllipticalArc> dialogTool = m_dialog.objectCast<DialogEllipticalArc>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    dialogTool->setArc(*elArc);
    dialogTool->SetCenter(elArc->GetCenter().id());
    dialogTool->SetF1(elArc->GetFormulaF1());
    dialogTool->SetF2(elArc->GetFormulaF2());
    dialogTool->SetRadius1(elArc->GetFormulaRadius1());
    dialogTool->SetRadius2(elArc->GetFormulaRadius2());
    dialogTool->SetRotationAngle(elArc->GetFormulaRotationAngle());
    dialogTool->setLineColor(elArc->getLineColor());
    dialogTool->setLineWeight(elArc->getLineWeight());
    dialogTool->setPenStyle(elArc->GetPenStyle());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool
 * @param dialog dialog options.
 * @param scene pointer to scene.
 * @param doc dom document container
 * @param data container with variables
 */
VToolEllipticalArc* VToolEllipticalArc::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                               VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogEllipticalArc> dialogTool = dialog.objectCast<DialogEllipticalArc>();
    SCASSERT(not dialogTool.isNull())

    const quint32 center = dialogTool->GetCenter();
    QString radius1          = dialogTool->GetRadius1();
    QString radius2          = dialogTool->GetRadius2();
    QString f1               = dialogTool->GetF1();
    QString f2               = dialogTool->GetF2();
    QString rotationAngle    = dialogTool->GetRotationAngle();
    const QString color      = dialogTool->getLineColor();
    const QString penStyle   = dialogTool->getPenStyle();
    const QString lineWeight = dialogTool->getLineWeight();
    VToolEllipticalArc* point = Create(0, center, radius1, radius2, f1, f2, rotationAngle, color, penStyle, lineWeight,
                                       scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param center id elliptical arc center point.
 * @param radius1 elliptical arc radius1.
 * @param radius2 elliptical arc radius2.
 * @param f1 start angle of elliptical arc.
 * @param f2 end angle of elliptical arc.
 * @param rotationAngle rotation angle of elliptical arc.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolEllipticalArc* VToolEllipticalArc::Create(const quint32 _id, const quint32 &center, QString &radius1,
                                               QString &radius2, QString &f1, QString &f2, QString &rotationAngle,
                                               const QString &color, const QString &penStyle, const QString &lineWeight,
                                               VMainGraphicsScene *scene, VAbstractPattern *doc,
                                               VContainer *data, const Document &parse, const Source &typeCreation)
{
    qreal calcRadius1 = 0, calcRadius2 = 0, calcF1 = 0, calcF2 = 0, calcRotationAngle = 0;

    calcRadius1 = qApp->toPixel(CheckFormula(_id, radius1, data));
    calcRadius2 = qApp->toPixel(CheckFormula(_id, radius2, data));

    calcF1 = CheckFormula(_id, f1, data);
    calcF2 = CheckFormula(_id, f2, data);
    calcRotationAngle = CheckFormula(_id, rotationAngle, data);

    const VPointF c = *data->GeometricObject<VPointF>(center);
    VEllipticalArc *elArc = new VEllipticalArc(c, calcRadius1, calcRadius2, radius1, radius2, calcF1, f1, calcF2, f2,
                                               calcRotationAngle, rotationAngle);
    elArc->setLineColor(color);
    elArc->SetPenStyle(penStyle);
    elArc->setLineWeight(lineWeight);
    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(elArc);
        data->AddArc(data->GeometricObject<VEllipticalArc>(id), id);
    }
    else
    {
        data->UpdateGObject(id, elArc);
        data->AddArc(data->GeometricObject<VEllipticalArc>(id), id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::EllipticalArc, doc);
        VToolEllipticalArc *toolEllipticalArc = new VToolEllipticalArc(doc, data, id, typeCreation);
        scene->addItem(toolEllipticalArc);
        InitElArcToolConnections(scene, toolEllipticalArc);
        VAbstractPattern::AddTool(id, toolEllipticalArc);
        doc->IncrementReferens(c.getIdTool());
        return toolEllipticalArc;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolEllipticalArc::getTagName() const
{
    return VAbstractPattern::TagElArc;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolEllipticalArc::CenterPointName() const
{
    return VAbstractTool::data.GetGObject(getCenter())->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolEllipticalArc::getCenter() const
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    return elArc->GetCenter().id();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::setCenter(const quint32 &value)
{
    if (value != NULL_ID)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);

        QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(value);
        elArc->SetCenter(*point.data());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolEllipticalArc::GetFormulaRadius1() const
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius1(elArc->GetFormulaRadius1(), getData());
    radius1.setCheckZero(true);
    radius1.setToolId(m_id);
    radius1.setPostfix(UnitsToStr(qApp->patternUnit()));
    return radius1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaRadius1(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0)// Formula don't check this, but radius1 can't be 0 or negative
        {
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
            elArc->SetFormulaRadius1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolEllipticalArc::GetFormulaRadius2() const
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius2(elArc->GetFormulaRadius2(), getData());
    radius2.setCheckZero(true);
    radius2.setToolId(m_id);
    radius2.setPostfix(UnitsToStr(qApp->patternUnit()));
    return radius2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaRadius2(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0)// Formula don't check this, but radius2 can't be 0 or negative
        {
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
            elArc->SetFormulaRadius2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolEllipticalArc::GetFormulaF1() const
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula f1(elArc->GetFormulaF1(), getData());
    f1.setCheckZero(false);
    f1.setToolId(m_id);
    f1.setPostfix(degreeSymbol);
    return f1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaF1(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);

        if (not VFuzzyComparePossibleNulls(value.getDoubleValue(), elArc->GetEndAngle()))// Angles can't be equal
        {
            elArc->SetFormulaF1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolEllipticalArc::GetFormulaF2() const
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula f2(elArc->GetFormulaF2(), getData());
    f2.setCheckZero(false);
    f2.setToolId(m_id);
    f2.setPostfix(degreeSymbol);
    return f2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaF2(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        if (not VFuzzyComparePossibleNulls(value.getDoubleValue(), elArc->GetStartAngle()))// Angles can't be equal
        {
            elArc->SetFormulaF2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolEllipticalArc::GetFormulaRotationAngle() const
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula rotationAngle(elArc->GetFormulaRotationAngle(), getData());
    rotationAngle.setCheckZero(false);
    rotationAngle.setToolId(m_id);
    rotationAngle.setPostfix(degreeSymbol);
    return rotationAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaRotationAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaRotationAngle(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolEllipticalArc>(show);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolEllipticalArc::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)

    try
    {
        ContextMenu<DialogEllipticalArc>(event);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolEllipticalArc::RemoveReferens()
{
    const auto elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    doc->DecrementReferens(elArc->GetCenter().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolEllipticalArc::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogEllipticalArc> dialogTool = m_dialog.objectCast<DialogEllipticalArc>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrCenter,        QString().setNum(dialogTool->GetCenter()));
    doc->SetAttribute(domElement, AttrRadius1,       dialogTool->GetRadius1());
    doc->SetAttribute(domElement, AttrRadius2,       dialogTool->GetRadius2());
    doc->SetAttribute(domElement, AttrAngle1,        dialogTool->GetF1());
    doc->SetAttribute(domElement, AttrAngle2,        dialogTool->GetF2());
    doc->SetAttribute(domElement, AttrRotationAngle, dialogTool->GetRotationAngle());
    doc->SetAttribute(domElement, AttrColor,         dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrPenStyle,      dialogTool->getPenStyle());
    doc->SetAttribute(domElement, AttrLineWeight,    dialogTool->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
    SCASSERT(elArc.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter,        elArc->GetCenter().id());
    doc->SetAttribute(tag, AttrRadius1,       elArc->GetFormulaRadius1());
    doc->SetAttribute(tag, AttrRadius2,       elArc->GetFormulaRadius2());
    doc->SetAttribute(tag, AttrAngle1,        elArc->GetFormulaF1());
    doc->SetAttribute(tag, AttrAngle2,        elArc->GetFormulaF2());
    doc->SetAttribute(tag, AttrRotationAngle, elArc->GetFormulaRotationAngle());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetVisualization()
{
    if (not vis.isNull())
    {
        const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
        VisToolEllipticalArc *visual = qobject_cast<VisToolEllipticalArc *>(vis);
        SCASSERT(visual != nullptr)

        const VTranslateVars *trVars = qApp->TrVars();
        visual->setObject1Id(elArc->GetCenter().id());
        visual->setRadius1(trVars->FormulaToUser(elArc->GetFormulaRadius1(), qApp->Settings()->GetOsSeparator()));
        visual->setRadius2(trVars->FormulaToUser(elArc->GetFormulaRadius2(), qApp->Settings()->GetOsSeparator()));
        visual->setF1(trVars->FormulaToUser(elArc->GetFormulaF1(), qApp->Settings()->GetOsSeparator()));
        visual->setF2(trVars->FormulaToUser(elArc->GetFormulaF2(), qApp->Settings()->GetOsSeparator()));
        visual->setRotationAngle(trVars->FormulaToUser(elArc->GetFormulaRotationAngle(),
                                                       qApp->Settings()->GetOsSeparator()));
        visual->setLineStyle(lineTypeToPenStyle(elArc->GetPenStyle()));
        visual->setLineWeight(elArc->getLineWeight());
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolEllipticalArc::makeToolTip() const
{
    const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);

    const QString toolTip = QString("<table>"
                                    "<tr> <td><b>%12:</b> %13</td> </tr>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                                    "<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                                    "<tr> <td><b>%8:</b> %9°</td> </tr>"
                                    "<tr> <td><b>%10:</b> %11°</td> </tr>"
                                    "</table>")
            .arg(tr("     Length"))
            .arg(qApp->fromPixel(elArc->GetLength()))
            .arg(UnitsToStr(qApp->patternUnit(), true))
            .arg(tr("    Radius") + QLatin1String("1"))
            .arg(qApp->fromPixel(elArc->GetRadius1()))
            .arg(tr("    Radius") + QLatin1String("2"))
            .arg(qApp->fromPixel(elArc->GetRadius2()))
            .arg(tr("Start angle"))
            .arg(qApp->fromPixel(elArc->GetStartAngle()))
            .arg(tr("  End angle"))
            .arg(qApp->fromPixel(elArc->GetEndAngle()))
            .arg(tr("      Label"))
            .arg(elArc->name());
    return toolTip;
}
