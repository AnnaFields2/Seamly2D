/***************************************************************************
 **  @file   vcontainer.cpp
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
 **
 **  @file   vcontainer.cpp
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

#include "vcontainer.h"

#include <limits.h>
#include <QVector>
#include <QtDebug>

#include "../ifc/exception/vexception.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "variables/varcradius.h"
#include "variables/vcurveangle.h"
#include "variables/vcurvelength.h"
#include "variables/vcurveclength.h"
#include "variables/vincrement.h"
#include "variables/vlineangle.h"
#include "variables/vlinelength.h"
#include "variables/measurement_variable.h"
#include "variables/vvariable.h"
#include "vtranslatevars.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vCon, "v.container")

QT_WARNING_POP

quint32 VContainer::_id = NULL_ID;
qreal VContainer::_size = 50;
qreal VContainer::_height = 176;
QSet<QString> VContainer::uniqueNames = QSet<QString>();

#ifdef Q_COMPILER_RVALUE_REFS
VContainer &VContainer::operator=(VContainer &&data) Q_DECL_NOTHROW
{ Swap(data); return *this; }
#endif

void VContainer::Swap(VContainer &data) Q_DECL_NOTHROW
{ std::swap(d, data.d); }

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VContainer create empty container
 */
VContainer::VContainer(const VTranslateVars *trVars, const Unit *patternUnit)
    :d(new VContainerData(trVars, patternUnit))
{}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief operator = copy constructor
 * @param data container
 * @return copy container
 */
VContainer &VContainer::operator =(const VContainer &data)
{
    if ( &data == this )
    {
        return *this;
    }
    d = data.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VContainer create container from another container
 * @param data container
 */
VContainer::VContainer(const VContainer &data)
    :d(data.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VContainer::~VContainer()
{
    ClearGObjects();
    ClearVariables();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetGObject returns a point by id
 * @param id id of point
 * @return point
 */
// cppcheck-suppress unusedFunction
const QSharedPointer<VGObject> VContainer::GetGObject(quint32 id)const
{
    return GetObject(d->gObjects, id);
}

//---------------------------------------------------------------------------------------------------------------------
const QSharedPointer<VGObject> VContainer::GetFakeGObject(quint32 id)
{
    VGObject *obj = new VGObject();
    obj->setId(id);
    QSharedPointer<VGObject> pointer(obj);
    return pointer;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetObject return object from container
 * @param obj container
 * @param id id of object
 * @return Object
 */
template <typename key, typename val>
const val VContainer::GetObject(const QHash<key, val> &obj, key id) const
{
    if (obj.contains(id))
    {
        return obj.value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find object: "), id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiece VContainer::GetPiece(quint32 id) const
{
    if (d->pieces->contains(id))
    {
        return d->pieces->value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find piece: "), id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath VContainer::GetPiecePath(quint32 id) const
{
    if (d->piecePaths->contains(id))
    {
        return d->piecePaths->value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find path: "), id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddGObject add new GObject to container
 * @param obj new object
 * @return return id of new object in container
 */
quint32 VContainer::AddGObject(VGObject *obj)
{
    SCASSERT(obj != nullptr)
    QSharedPointer<VGObject> pointer(obj);
    uniqueNames.insert(obj->name());
    return AddObject(d->gObjects, pointer);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VContainer::AddPiece(const VPiece &piece)
{
    const quint32 id = getNextId();
    d->pieces->insert(id, piece);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VContainer::AddPiecePath(const VPiecePath &path)
{
    const quint32 id = getNextId();
    d->piecePaths->insert(id, path);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VContainer::getId()
{
    return _id;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getNextId generate next unique id
 * @return next unique id
 */
quint32 VContainer::getNextId()
{
    //TODO. Current count of ids are very big and allow us save time before someone will reach its max value.
    //Better way, of cource, is to seek free ids inside the set of values and reuse them.
    //But for now better to keep it as it is now.
    if (_id == UINT_MAX)
    {
        qCritical() << (tr("Number of free id exhausted."));
    }
    _id++;
    return _id;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateId update id. If new id bigger when current save new like current.
 * @param newId id
 */
void VContainer::UpdateId(quint32 newId)
{
    if (newId > _id)
    {
       _id = newId;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Clear clear data in container. Id will be 0.
 */
void VContainer::Clear()
{
    qCDebug(vCon, "Clearing container data.");
    _id = NULL_ID;

    d->pieces->clear();
    d->piecePaths->clear();
    ClearVariables();
    ClearGObjects();
    ClearUniqueNames();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearForFullParse()
{
    qCDebug(vCon, "Clearing container data for full parse.");
    _id = NULL_ID;

    d->pieces->clear();
    d->piecePaths->clear();
    ClearVariables(VarType::Increment);
    ClearVariables(VarType::LineAngle);
    ClearVariables(VarType::LineLength);
    ClearVariables(VarType::CurveLength);
    ClearVariables(VarType::CurveCLength);
    ClearVariables(VarType::ArcRadius);
    ClearVariables(VarType::CurveAngle);
    ClearGObjects();
    ClearUniqueNames();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClearObject points, splines, arcs, spline paths will be cleared.
 */
void VContainer::ClearGObjects()
{
    d->gObjects.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearCalculationGObjects()
{
    if (not d->gObjects.isEmpty()) //-V807
    {
        QVector<quint32> keys;
        QHash<quint32, QSharedPointer<VGObject> >::iterator i;
        for (i = d->gObjects.begin(); i != d->gObjects.end(); ++i)
        {
            if (i.value()->getMode() == Draw::Calculation)
            {
                i.value().clear();
                keys.append(i.key());
            }
        }
        // We can't delete objects in previous loop it will destroy the iterator.
        if (not keys.isEmpty())
        {
            for (int i = 0; i < keys.size(); ++i)
            {
                d->gObjects.remove(keys.at(i));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearVariables(const VarType &type)
{
    if (d->variables.size()>0) //-V807
    {
        if (type == VarType::Unknown)
        {
            d->variables.clear();
        }
        else
        {
            QVector<QString> keys;
            QHash<QString, QSharedPointer<VInternalVariable> >::iterator i;
            for (i = d->variables.begin(); i != d->variables.end(); ++i)
            {
                if (i.value()->GetType() == type)
                {
                    keys.append(i.key());
                }
            }

            for (int i = 0; i < keys.size(); ++i)
            {
                d->variables.remove(keys.at(i));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddLine add line to container
 * @param firstPointId id of first point of line
 * @param secondPointId id of second point of line
 */
void VContainer::AddLine(const quint32 &firstPointId, const quint32 &secondPointId)
{
    const QSharedPointer<VPointF> first = GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> second = GeometricObject<VPointF>(secondPointId);

    VLengthLine *length = new VLengthLine(first.data(), firstPointId, second.data(), secondPointId, *GetPatternUnit());
    AddVariable(length->GetName(), length);

    VLineAngle *angle = new VLineAngle(first.data(), firstPointId, second.data(), secondPointId);
    AddVariable(angle->GetName(), angle);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddArc(const QSharedPointer<VAbstractCurve> &arc, const quint32 &id, const quint32 &parentId)
{
    AddCurve(arc, id, parentId);

    if (arc->getType() == GOType::Arc)
    {
        const QSharedPointer<VArc> casted = arc.staticCast<VArc>();

        VArcRadius *radius = new VArcRadius(id, parentId, casted.data(), *GetPatternUnit());
        AddVariable(radius->GetName(), radius);
    }
    else if (arc->getType() == GOType::EllipticalArc)
    {
        const QSharedPointer<VEllipticalArc> casted = arc.staticCast<VEllipticalArc>();

        VArcRadius *radius1 = new VArcRadius(id, parentId, casted.data(), 1, *GetPatternUnit());
        AddVariable(radius1->GetName(), radius1);

        VArcRadius *radius2 = new VArcRadius(id, parentId, casted.data(), 2, *GetPatternUnit());
        AddVariable(radius2->GetName(), radius2);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId)
{
    const GOType curveType = curve->getType();
    if (curveType != GOType::Spline      && curveType != GOType::SplinePath &&
        curveType != GOType::CubicBezier && curveType != GOType::CubicBezierPath &&
        curveType != GOType::Arc         && curveType != GOType::EllipticalArc)
    {
        throw VException(tr("Can't create a curve with type '%1'").arg(static_cast<int>(curveType)));
    }

    VCurveLength *length = new VCurveLength(id, parentId, curve.data(), *GetPatternUnit());
    AddVariable(length->GetName(), length);

    VCurveAngle *startAngle = new VCurveAngle(id, parentId, curve.data(), CurveAngle::StartAngle);
    AddVariable(startAngle->GetName(), startAngle);

    VCurveAngle *endAngle = new VCurveAngle(id, parentId, curve.data(), CurveAngle::EndAngle);
    AddVariable(endAngle->GetName(), endAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddSpline(const QSharedPointer<VAbstractBezier> &curve, quint32 id, quint32 parentId)
{
    AddCurve(curve, id, parentId);

    VCurveCLength *c1Length = new VCurveCLength(id, parentId, curve.data(), CurveCLength::C1, *GetPatternUnit());
    AddVariable(c1Length->GetName(), c1Length);

    VCurveCLength *c2Length = new VCurveCLength(id, parentId, curve.data(), CurveCLength::C2, *GetPatternUnit());
    AddVariable(c2Length->GetName(), c2Length);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::AddCurveWithSegments(const QSharedPointer<VAbstractCubicBezierPath> &curve, const quint32 &id,
                                      quint32 parentId)
{
    AddSpline(curve, id, parentId);

    for (qint32 i = 1; i <= curve->CountSubSpl(); ++i)
    {
        const VSpline spl = curve->GetSpline(i);

        VCurveLength *length = new VCurveLength(id, parentId, curve->name(), spl, *GetPatternUnit(), i);
        AddVariable(length->GetName(), length);

        VCurveAngle *startAngle = new VCurveAngle(id, parentId, curve->name(), spl, CurveAngle::StartAngle, i);
        AddVariable(startAngle->GetName(), startAngle);

        VCurveAngle *endAngle = new VCurveAngle(id, parentId, curve->name(), spl, CurveAngle::EndAngle, i);
        AddVariable(endAngle->GetName(), endAngle);

        VCurveCLength *c1Length = new VCurveCLength(id, parentId, curve->name(), spl, CurveCLength::C1,
                                                    *GetPatternUnit(), i);
        AddVariable(c1Length->GetName(), c1Length);

        VCurveCLength *c2Length = new VCurveCLength(id, parentId, curve->name(), spl, CurveCLength::C2,
                                                    *GetPatternUnit(), i);
        AddVariable(c2Length->GetName(), c2Length);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::RemoveVariable(const QString &name)
{
    d->variables.remove(name);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::RemovePiece(quint32 id)
{
    d->pieces->remove(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddObject add object to container
 * @param obj container
 * @param value object
 * @return id of object in container
 */
template <typename key, typename val>
quint32 VContainer::AddObject(QHash<key, val> &obj, val value)
{
    SCASSERT(value != nullptr)
    const quint32 id = getNextId();
    value->setId(id);
    obj[id] = value;
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::UpdatePiece(quint32 id, const VPiece &piece)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    d->pieces->insert(id, piece);
    UpdateId(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::UpdatePiecePath(quint32 id, const VPiecePath &path)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    d->piecePaths->insert(id, path);
    UpdateId(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief removeCustomVariable remove increment by name from increment table
 * @param name name of existing increment
 */
void VContainer::removeCustomVariable(const QString &name)
{
    d->variables[name].clear();
    d->variables.remove(name);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<MeasurementVariable> > VContainer::DataMeasurements() const
{
    return DataVar<MeasurementVariable>(VarType::Measurement);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VIncrement> > VContainer::variablesData() const
{
    return DataVar<VIncrement>(VarType::Increment);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VLengthLine> > VContainer::lineLengthsData() const
{
    return DataVar<VLengthLine>(VarType::LineLength);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VCurveLength> > VContainer::curveLengthsData() const
{
    return DataVar<VCurveLength>(VarType::CurveLength);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VCurveCLength> > VContainer::controlPointLengthsData() const
{
    return DataVar<VCurveCLength>(VarType::CurveCLength);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VLineAngle> > VContainer::lineAnglesData() const
{
    return DataVar<VLineAngle>(VarType::LineAngle);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VArcRadius> > VContainer::arcRadiusesData() const
{
    return DataVar<VArcRadius>(VarType::ArcRadius);
}

//---------------------------------------------------------------------------------------------------------------------
const QMap<QString, QSharedPointer<VCurveAngle> > VContainer::curveAnglesData() const
{
    return DataVar<VCurveAngle>(VarType::CurveAngle);
}

//---------------------------------------------------------------------------------------------------------------------
bool VContainer::IsUnique(const QString &name)
{
    return (!uniqueNames.contains(name) && !builInFunctions.contains(name));
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VContainer::AllUniqueNames()
{
    QStringList names = builInFunctions;
	names.append(uniqueNames.values());
    return names;
}

//---------------------------------------------------------------------------------------------------------------------
const Unit *VContainer::GetPatternUnit() const
{
    return d->patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
const VTranslateVars *VContainer::GetTrVars() const
{
    return d->trVars;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
const QMap<QString, QSharedPointer<T> > VContainer::DataVar(const VarType &type) const
{
    QMap<QString, QSharedPointer<T> > map;
    //Sorting QHash by id
    QHash<QString, QSharedPointer<VInternalVariable> >::const_iterator i;
    for (i = d->variables.constBegin(); i != d->variables.constEnd(); ++i)
    {
        if (i.value()->GetType() == type)
        {
            QSharedPointer<T> var = GetVariable<T>(i.key());
            map.insert(d->trVars->VarToUser(i.key()), var);
        }
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearUniqueNames()
{
    uniqueNames.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VContainer::ClearUniqueIncrementNames()
{
	const QList<QString> list = uniqueNames.values();
    ClearUniqueNames();

    for(int i = 0; i < list.size(); ++i)
    {
        if (not list.at(i).startsWith('#'))
        {
            uniqueNames.insert(list.at(i));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSize set value of size
 * @param size value of size
 */
void VContainer::setSize(qreal size)
{
    _size = size;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetGrowth set value of growth
 * @param height value of height
 */
void VContainer::setHeight(qreal height)
{
    _height = height;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief size return size
 * @return size in mm
 */
qreal VContainer::size()
{
    return _size;
}

//---------------------------------------------------------------------------------------------------------------------
qreal *VContainer::rsize()
{
    return &_size;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief height return height
 * @return height in pattern units
 */
qreal VContainer::height()
{
    return _height;
}

//---------------------------------------------------------------------------------------------------------------------
qreal *VContainer::rheight()
{
    return &_height;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief data container with datagObjects return container of gObjects
 * @return pointer on container of gObjects
 */
const QHash<quint32, QSharedPointer<VGObject> > *VContainer::DataGObjects() const
{
    return &d->gObjects;
}

//---------------------------------------------------------------------------------------------------------------------
const QHash<quint32, VPiece> *VContainer::DataPieces() const
{
    return d->pieces.data();
}

//---------------------------------------------------------------------------------------------------------------------
const QHash<QString, QSharedPointer<VInternalVariable> > *VContainer::DataVariables() const
{
    return &d->variables;
}

//---------------------------------------------------------------------------------------------------------------------
VContainerData::~VContainerData()
{}
