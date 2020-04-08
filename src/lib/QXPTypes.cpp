/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QXPTypes.h"

#include <boost/math/constants/constants.hpp>
#include <cmath>

namespace libqxp
{

bool operator==(const Point &lhs, const Point &rhs)
{
  return QXP_ALMOST_ZERO(lhs.x - rhs.x) && QXP_ALMOST_ZERO(lhs.y - rhs.y);
}

bool operator!=(const Point &lhs, const Point &rhs)
{
  return !(lhs == rhs);
}

Point Point::move(double dx, double dy) const
{
  return Point(x + dx, y + dy);
}

Point Point::rotateDeg(double rotationDeg, const Point &center) const
{
  if (QXP_ALMOST_ZERO(rotationDeg))
  {
    return *this;
  }
  const double rotation = deg2rad(rotationDeg);
  const double rotatedX = (x - center.x) * std::cos(rotation) - (y - center.y) * std::sin(rotation) + center.x;
  const double rotatedY = (y - center.y) * std::cos(rotation) + (x - center.x) * std::sin(rotation) + center.y;
  return Point(rotatedX, rotatedY);
}

double Point::distance(const Point &p2) const
{
  return std::hypot(p2.x - x, p2.y - y);
}

Rect::Rect()
  : top(0.0), right(0.0), bottom(0.0), left(0.0)
{ }

Rect::Rect(double t, double r, double b, double l)
  : top(t), right(r), bottom(b), left(l)
{ }

double Rect::width() const
{
  return right - left;
}

double Rect::height() const
{
  return bottom - top;
}

Point Rect::center() const
{
  return topLeft().move(width() / 2, height() / 2);
}

Point Rect::topLeft() const
{
  return Point(left, top);
}

Point Rect::topRight() const
{
  return Point(right, top);
}

Point Rect::bottomRight() const
{
  return Point(right, bottom);
}

Point Rect::bottomLeft() const
{
  return Point(left, bottom);
}

Rect Rect::shrink(const double diff) const
{
  return Rect(top + diff, right - diff, bottom - diff, left + diff);
}

librevenge::RVNGString Color::toString() const
{
  librevenge::RVNGString colorStr;
  colorStr.sprintf("#%.2x%.2x%.2x", red, green, blue);
  return colorStr;
}

Color Color::applyShade(double shade) const
{
  if (shade < 0.0 || shade > 1.0)
  {
    QXP_DEBUG_MSG(("Invalid shade %lf\n", shade));
    return *this;
  }
  const double tint = 1 - shade;
  return Color(uint8_t(std::round(red + (255 - red) * tint)),
               uint8_t(std::round(green + (255 - green) * tint)),
               uint8_t(std::round(blue + (255 - blue) * tint)));
}

bool TextSpec::overlaps(const TextSpec &other) const
{
  return startIndex <= other.endIndex() && other.startIndex <= endIndex();
}

double Text::maxFontSize() const
{
  if (charFormats.empty())
  {
    QXP_DEBUG_MSG(("Text::maxFontSize no char formats\n"));
  }

  double maxSize = 0;
  for (auto &charFormat : charFormats)
  {
    if (!charFormat.format->isControlChars && charFormat.format->fontSize > maxSize)
    {
      maxSize = charFormat.format->fontSize;
    }
  }
  return maxSize;
}

double Text::maxFontSize(const ParagraphSpec &paragraph) const
{
  double maxSize = 0;
  for (auto &charFormat : charFormats)
  {
    if (!charFormat.format->isControlChars && charFormat.overlaps(paragraph) && charFormat.format->fontSize > maxSize)
    {
      maxSize = charFormat.format->fontSize;
    }
  }
  return maxSize;
}

bool TextObject::isLinked() const
{
  return linkSettings.linkedIndex > 0 || linkSettings.nextLinkedIndex > 0;
}

void QXPDocumentProperties::setAutoLeading(const double val)
{
  if (val >= -63 && val <= 63)
  {
    m_autoLeading = val;
  }
  else
  {
    QXP_DEBUG_MSG(("Invalid auto leading %lf\n", val));
  }
}

double QXPDocumentProperties::autoLeading() const
{
  return m_autoLeading;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
