/***************************************************************************
 **  @file   vtoolnormal.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   vtoolnormal.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vtoolnormal.h"

#include <QLineF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../../dialogs/tools/dialognormal.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../../visualization/line/vistoolnormal.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

const QString VToolNormal::ToolType = QStringLiteral("normal");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolNormal constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param formula string with formula normal length.
 * @param angle additional angle.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolNormal::VToolNormal(VAbstractPattern *doc, VContainer *data, const quint32 &id, const QString &lineType,
                         const QString &lineWeight, const QString &lineColor, const QString &formula,
                         const qreal &angle,
                         const quint32 &firstPointId, const quint32 &secondPointId, const Source &typeCreation,
                         QGraphicsItem *parent)
    :VToolLinePoint(doc, data, id, lineType, lineWeight, lineColor, formula, firstPointId, angle, parent),
    secondPointId(secondPointId)
{

    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolNormal::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogNormal> dialogTool = m_dialog.objectCast<DialogNormal>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->setLineColor(lineColor);
    dialogTool->SetFormula(formulaLength);
    dialogTool->SetAngle(angle);
    dialogTool->SetFirstPointId(basePointId);
    dialogTool->SetSecondPointId(secondPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
VToolNormal* VToolNormal::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogNormal> dialogTool = dialog.objectCast<DialogNormal>();
    SCASSERT(not dialogTool.isNull())
    QString formula = dialogTool->GetFormula();
    const quint32 firstPointId  = dialogTool->GetFirstPointId();
    const quint32 secondPointId = dialogTool->GetSecondPointId();
    const QString lineType      = dialogTool->getLineType();
    const QString lineWeight    = dialogTool->getLineWeight();
    const QString lineColor     = dialogTool->getLineColor();
    const QString pointName     = dialogTool->getPointName();
    const qreal   angle         = dialogTool->GetAngle();
    VToolNormal *point = Create(0, formula, firstPointId, secondPointId, lineType, lineWeight, lineColor, pointName, angle,
                                5, 10, true, scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param formula string with formula normal length.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param pointName point name.
 * @param angle additional angle.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param showPointName show/hide point name text.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolNormal* VToolNormal::Create(const quint32 _id, QString &formula, quint32 firstPointId,
                                 quint32 secondPointId, const QString &lineType, const QString &lineWeight,
                                 const QString &lineColor,
                                 const QString &pointName, qreal angle, qreal mx, qreal my, bool showPointName,
                                 VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                 const Document &parse,
                                 const Source &typeCreation)
{
    const QSharedPointer<VPointF> firstPoint = data->GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondPoint = data->GeometricObject<VPointF>(secondPointId);

    const qreal result = CheckFormula(_id, formula, data);

    QPointF fPoint = VToolNormal::FindPoint(static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint),
                                            qApp->toPixel(result), angle);
    quint32 id = _id;
    VPointF *p = new VPointF(fPoint, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(firstPointId, id);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(firstPointId, id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::Normal, doc);
        VToolNormal *point = new VToolNormal(doc, data, id, lineType, lineWeight, lineColor, formula, angle, firstPointId,
                                             secondPointId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(firstPoint->getIdTool());
        doc->IncrementReferens(secondPoint->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint return normal point.
 * @param firstPoint first line point.
 * @param secondPoint second line point.
 * @param length normal length.
 * @param angle additional angle.
 * @return normal point.
 */
QPointF VToolNormal::FindPoint(const QPointF &firstPoint, const QPointF &secondPoint, const qreal &length,
                               const qreal &angle)
{
    QLineF line(firstPoint, secondPoint);
    QLineF normal = line.normalVector();
    normal.setAngle(normal.angle()+angle);
    normal.setLength(length);
    return normal.p2();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolNormal::SecondPointName() const
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolNormal::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogNormal>(event, id);
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
void VToolNormal::RemoveReferens()
{
    const auto secondPoint = VAbstractTool::data.GetGObject(secondPointId);
    doc->DecrementReferens(secondPoint->getIdTool());
    VToolLinePoint::RemoveReferens();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolNormal::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogNormal> dialogTool = m_dialog.objectCast<DialogNormal>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,        dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrLineType,    dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight,  dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,   dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrLength,      dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrAngle,       QString().setNum(dialogTool->GetAngle()));
    doc->SetAttribute(domElement, AttrFirstPoint,  QString().setNum(dialogTool->GetFirstPointId()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPointId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formulaLength);
    doc->SetAttribute(tag, AttrAngle, angle);
    doc->SetAttribute(tag, AttrFirstPoint, basePointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::ReadToolAttributes(const QDomElement &domElement)
{
    m_lineType    = doc->GetParametrString(domElement, AttrLineType, LineTypeSolidLine);
    m_lineWeight  = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor     = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    formulaLength = doc->GetParametrString(domElement, AttrLength, "");
    basePointId   = doc->GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPointId = doc->GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
    angle         = doc->GetParametrDouble(domElement, AttrAngle, "0");
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolNormal *visual = qobject_cast<VisToolNormal *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(basePointId);
        visual->setObject2Id(secondPointId);
        visual->setLength(qApp->TrVars()->FormulaToUser(formulaLength, qApp->Settings()->GetOsSeparator()));
        visual->SetAngle(angle);
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolNormal::GetSecondPointId() const
{
    return secondPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::SetSecondPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolNormal>(show);
}
