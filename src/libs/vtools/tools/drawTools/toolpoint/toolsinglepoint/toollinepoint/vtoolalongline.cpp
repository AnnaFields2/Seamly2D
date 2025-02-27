/***************************************************************************
 **  @file   vtoolalongline.cpp
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
 **  @file   vtoolalongline.cpp
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

#include "vtoolalongline.h"

#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../../dialogs/tools/dialogalongline.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../../visualization/line/vistoolalongline.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/variables/vlinelength.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

const QString VToolAlongLine::ToolType = QStringLiteral("alongLine");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolAlongLine constuctor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param formula string with length formula.
 * @param firstPointId id first point of line.
 * @param secondPointId id second point of line.
 * @param lineType line type. line type.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolAlongLine::VToolAlongLine(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &formula,
                               const quint32 &firstPointId, const quint32 &secondPointId,
                               const QString &lineType, const QString &lineWeight, const QString &lineColor,
                               const Source &typeCreation, QGraphicsItem *parent)
    :VToolLinePoint(doc, data, id, lineType, lineWeight, lineColor, formula, firstPointId, 0, parent), secondPointId(secondPointId)
{
    m_pointColor = QColor(lineColor);
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events. handle context menu event.
 * @param event context menu event.
 */
//cppcheck-suppress unusedFunction
void VToolAlongLine::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogAlongLine>(event, id);
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
void VToolAlongLine::RemoveReferens()
{
    const auto secondPoint = VAbstractTool::data.GetGObject(secondPointId);
    doc->DecrementReferens(secondPoint->getIdTool());
    VToolLinePoint::RemoveReferens();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolAlongLine::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogAlongLine> dialogTool = m_dialog.objectCast<DialogAlongLine>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,        dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrLineType,    dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight,  dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,   dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrLength,      dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrFirstPoint,  dialogTool->GetFirstPointId());
    doc->SetAttribute(domElement, AttrSecondPoint, dialogTool->GetSecondPointId());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAlongLine::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType,        ToolType);
    doc->SetAttribute(tag, AttrLength,      formulaLength);
    doc->SetAttribute(tag, AttrFirstPoint,  basePointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAlongLine::ReadToolAttributes(const QDomElement &domElement)
{
    m_lineType    = doc->GetParametrString(domElement, AttrLineType,    LineTypeSolidLine);
    m_lineWeight  = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor     = doc->GetParametrString(domElement, AttrLineColor,   ColorBlack);
    formulaLength = doc->GetParametrString(domElement, AttrLength,      "");
    basePointId   = doc->GetParametrUInt(domElement,   AttrFirstPoint,  NULL_ID_STR);
    secondPointId = doc->GetParametrUInt(domElement,   AttrSecondPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAlongLine::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolAlongLine *visual = qobject_cast<VisToolAlongLine *>(vis);
        SCASSERT(visual != nullptr)
        visual->setObject1Id(basePointId);
        visual->setObject2Id(secondPointId);
        visual->setLength(qApp->TrVars()->FormulaToUser(formulaLength, qApp->Settings()->GetOsSeparator()));
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolAlongLine::makeToolTip() const
{
    const QSharedPointer<VPointF> basePoint = VAbstractTool::data.GeometricObject<VPointF>(basePointId);
    const QSharedPointer<VPointF> secondPoint = VAbstractTool::data.GeometricObject<VPointF>(secondPointId);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF curLine(static_cast<QPointF>(*basePoint), static_cast<QPointF>(*current));
    const QLineF curToSecond(static_cast<QPointF>(*current), static_cast<QPointF>(*secondPoint));

    const QString toolTip = QString("<table>"
                                    "<tr> <td><b>  %9:</b> %10</td> </tr>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b> %4:</b> %5°</td> </tr>"
                                    "<tr> <td><b>%6:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b>%7:</b> %8 %3</td> </tr>"
                                    "</table>")
            .arg(tr("Length"))
            .arg(qApp->fromPixel(curLine.length()))
            .arg(UnitsToStr(qApp->patternUnit(), true))
            .arg(tr("Angle"))
            .arg(curLine.angle())
            .arg(QString("%1->%2").arg(basePoint->name(), current->name()))
            .arg(QString("%1->%2").arg(current->name(), secondPoint->name()))
            .arg(qApp->fromPixel(curToSecond.length()))
            .arg(tr("Name"))
            .arg(current->name());

    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolAlongLine::GetSecondPointId() const
{
    return secondPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAlongLine::SetSecondPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAlongLine::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolAlongLine>(show);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolAlongLine::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogAlongLine> dialogTool = m_dialog.objectCast<DialogAlongLine>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->setLineColor(lineColor);
    dialogTool->SetFormula(formulaLength);
    dialogTool->SetFirstPointId(basePointId);
    dialogTool->SetSecondPointId(secondPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog options.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
VToolAlongLine* VToolAlongLine::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                       VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogAlongLine> dialogTool = dialog.objectCast<DialogAlongLine>();
    SCASSERT(not dialogTool.isNull())
    QString formula             = dialogTool->GetFormula();
    const quint32 firstPointId  = dialogTool->GetFirstPointId();
    const quint32 secondPointId = dialogTool->GetSecondPointId();
    const QString lineType      = dialogTool->getLineType();
    const QString lineWeight    = dialogTool->getLineWeight();
    const QString lineColor     = dialogTool->getLineColor();
    const QString pointName     = dialogTool->getPointName();
    VToolAlongLine *point = Create(0, pointName, lineType, lineWeight, lineColor, formula, firstPointId, secondPointId,
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
 * @param pointName point name. point name.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param formula string with length formula.
 * @param firstPointId id first point of line.
 * @param secondPointId id second point of line.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolAlongLine* VToolAlongLine::Create(const quint32 _id, const QString &pointName, const QString &lineType,
                                       const QString &lineWeight,
                                       const QString &lineColor, QString &formula, const quint32 &firstPointId,
                                       quint32 secondPointId, qreal mx, qreal my, bool showPointName,
                                       VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                       const Document &parse, const Source &typeCreation)
{
    const QSharedPointer<VPointF> firstPoint = data->GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondPoint = data->GeometricObject<VPointF>(secondPointId);
    QLineF line = QLineF(static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint));

    //Declare special variable "CurrentLength"
    VLengthLine *length = new VLengthLine(firstPoint.data(), firstPointId, secondPoint.data(),
                                          secondPointId, *data->GetPatternUnit());
    length->SetName(currentLength);
    data->AddVariable(currentLength, length);

    line.setLength(qApp->toPixel(CheckFormula(_id, formula, data)));

    quint32 id = _id;
    VPointF *p = new VPointF(line.p2(), pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(firstPointId, id);
        data->AddLine(id, secondPointId);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(firstPointId, id);
        data->AddLine(id, secondPointId);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    VToolAlongLine *point = nullptr;
    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::AlongLine, doc);
        point = new VToolAlongLine(doc, data, id, formula, firstPointId, secondPointId, lineType, lineWeight, lineColor,
                                   typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(firstPoint->getIdTool());
        doc->IncrementReferens(secondPoint->getIdTool());
    }
    //Very important to delete it. Only this tool need this special variable.
    data->RemoveVariable(currentLength);
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolAlongLine::SecondPointName() const
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}
