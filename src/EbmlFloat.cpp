/****************************************************************************
** libebml : parse EBML files, see http://embl.sourceforge.net/
**
** <file/class description>
**
** Copyright (C) 2002-2010 Steve Lhomme.  All rights reserved.
**
** This file is part of libebml.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** See http://www.gnu.org/licenses/lgpl-2.1.html for LGPL licensing information.
**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
  \file
  \version \$Id$
  \author Steve Lhomme     <robux4 @ users.sf.net>
*/

#include <cassert>

#include "ebml/EbmlFloat.h"

namespace libebml {

EbmlFloat::EbmlFloat(const EbmlFloat::Precision prec)
  :EbmlElement(0, false)
{
  SetPrecision(prec);
}

EbmlFloat::EbmlFloat(const double aDefaultValue, const EbmlFloat::Precision prec)
  :EbmlElement(0, true), Value(aDefaultValue), DefaultValue(aDefaultValue)
{
  SetDefaultIsSet();
  SetPrecision(prec);
}

void EbmlFloat::SetDefaultValue(double aValue)
{
  assert(!DefaultISset());
  DefaultValue = aValue;
  SetDefaultIsSet();
}

double EbmlFloat::DefaultVal() const
{
  assert(DefaultISset());
  return DefaultValue;
}

EbmlFloat::operator float() const {return static_cast<float>(Value);}
EbmlFloat::operator double() const {return (Value);}

double EbmlFloat::GetValue() const {return Value;}

EbmlFloat & EbmlFloat::SetValue(double NewValue) {
  return *this = NewValue;
}

/*!
  \todo handle exception on errors
  \todo handle 10 bits precision
*/
filepos_t EbmlFloat::RenderData(IOCallback & output, bool /* bForceRender */, bool /* bWithDefault */)
{
  assert(GetSize() == 4 || GetSize() == 8);

  if (GetSize() == 4) {
    auto val = static_cast<float>(Value);
    std::int32_t Tmp;
    memcpy(&Tmp, &val, 4);
    binary TmpToWrite[4];
    endian::to_big32(Tmp, TmpToWrite);
    output.writeFully(TmpToWrite, 4);
  } else if (GetSize() == 8) {
    double val = Value;
    std::int64_t Tmp;
    memcpy(&Tmp, &val, 8);
    binary TmpToWrite[8];
    endian::to_big64(Tmp, TmpToWrite);
    output.writeFully(TmpToWrite, 8);
  }

  return GetSize();
}

std::uint64_t EbmlFloat::UpdateSize(bool bWithDefault, bool  /* bForceRender */)
{
  if (!bWithDefault && IsDefaultValue())
    return 0;
  return GetSize();
}

/*!
  \todo remove the hack for possible endianess pb (test on little & big endian)
*/
filepos_t EbmlFloat::ReadData(IOCallback & input, ScopeMode ReadFully)
{
  if (ReadFully == SCOPE_NO_DATA)
    return GetSize();

  assert(GetSize() == 4 || GetSize() == 8);
  if (GetSize() != 4 && GetSize() != 8) {
    // impossible to read, skip it
    input.setFilePointer(GetSize(), seek_current);
    return GetSize();
  }

  binary Buffer[8];
  input.readFully(Buffer, GetSize());

  if (GetSize() == 4) {
    auto tmpp = endian::from_big32(Buffer);
    float val;
    memcpy(&val, &tmpp, 4);
    Value = static_cast<double>(val);
  } else {
    auto tmpp = endian::from_big64(Buffer);
    memcpy(&Value, &tmpp, 8);
  }
  SetValueIsSet();

  return GetSize();
}

bool EbmlFloat::IsSmallerThan(const EbmlElement *Cmp) const
{
  if (EbmlId(*this) == EbmlId(*Cmp))
    return this->Value < static_cast<const EbmlFloat *>(Cmp)->Value;

  return false;
}

} // namespace libebml
