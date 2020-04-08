/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libqxp project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QXPCONTENTCOLLECTOR_H_INCLUDED
#define QXPCONTENTCOLLECTOR_H_INCLUDED

#include "QXPCollector.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <type_traits>

#include "QXPTypes.h"

namespace libqxp
{

class QXPContentCollector : public QXPCollector
{
  // disable copying
  QXPContentCollector(const QXPContentCollector &other) = delete;
  QXPContentCollector &operator=(const QXPContentCollector &other) = delete;

public:
  QXPContentCollector(librevenge::RVNGDrawingInterface *painter);
  ~QXPContentCollector();

  void startDocument() override;
  void endDocument() override;

  void startPage(const Page &page) override;
  void endPage() override;

  void collectDocumentProperties(const QXPDocumentProperties &props) override;

  void collectLine(const std::shared_ptr<Line> &line) override;
  void collectBox(const std::shared_ptr<Box> &box) override;
  void collectTextBox(const std::shared_ptr<TextBox> &box) override;
  void collectTextPath(const std::shared_ptr<TextPath> &textPath) override;
  void collectGroup(const std::shared_ptr<Group> &group) override;

  void collectText(const std::shared_ptr<Text> &text, const unsigned linkId) override;

private:
  struct CollectedPage;

  template<typename T>
  using ObjectHandler = std::function<void(const std::shared_ptr<T> &obj, const CollectedPage &)>;

  template<typename T, typename TThis>
  using ObjectHandlerMember = std::function<void(TThis *, const std::shared_ptr<T> &obj, const CollectedPage &)>;

  class CollectedObjectInterface
  {
  public:
    virtual ~CollectedObjectInterface() = default;

    virtual void draw(const CollectedPage &page) = 0;

    virtual unsigned zIndex() const = 0;
    virtual void setZIndex(unsigned value) = 0;
  };

  template<typename T>
  class CollectedObject : public CollectedObjectInterface
  {
    static_assert(std::is_base_of<Object, T>::value, "T is not Object");
  public:
    const std::shared_ptr<T> object;

    CollectedObject(const std::shared_ptr<T> &obj, const ObjectHandler<T> &handler)
      : object(obj), m_handler(handler), m_isProcessed(false)
    { }

    void draw(const CollectedPage &page) override
    {
      if (!m_isProcessed)
      {
        m_isProcessed = true;
        m_handler(object, page);
      }
    }

    unsigned zIndex() const override
    {
      return object->zIndex;
    }

    void setZIndex(unsigned value) override
    {
      object->zIndex = value;
    }

  private:
    const ObjectHandler<T> m_handler;
    bool m_isProcessed;
  };

  struct CollectedPage
  {
    const PageSettings settings;
    std::vector<std::shared_ptr<CollectedObject<Group>>> groups;
    std::vector<std::shared_ptr<TextObject>> linkedTextObjects;
    std::map<unsigned, std::shared_ptr<CollectedObjectInterface>> objects;

    CollectedPage(const PageSettings &pageSettings)
      : settings(pageSettings), groups(), linkedTextObjects(), objects()
    { }

    double getX(const double x) const;
    double getX(const std::shared_ptr<Object> &obj) const;
    double getY(const double y) const;
    double getY(const std::shared_ptr<Object> &obj) const;
    Point getPoint(const Point &p) const;
  };

  librevenge::RVNGDrawingInterface *m_painter;

  bool m_isDocumentStarted;
  bool m_isCollectingFacingPage;
  unsigned m_currentObjectIndex;

  std::vector<CollectedPage> m_unprocessedPages;

  std::unordered_map<unsigned, std::shared_ptr<Text>> m_linkTextMap;
  std::unordered_map<unsigned, std::unordered_map<unsigned, std::shared_ptr<TextObject>>> m_linkIndexedTextObjectsMap;

  QXPDocumentProperties m_docProps;

  CollectedPage &getInsertionPage(const std::shared_ptr<Object> &obj);

  template<typename T>
  std::shared_ptr<CollectedObject<T>> addObject(const std::shared_ptr<T> &obj, const ObjectHandlerMember<T, QXPContentCollector> &handler)
  {
    auto collectedObj = std::make_shared<CollectedObject<T>>(obj, std::bind(handler, this, std::placeholders::_1, std::placeholders::_2));
    getInsertionPage(obj).objects[m_currentObjectIndex] = collectedObj;
    ++m_currentObjectIndex;
    return collectedObj;
  }

  void draw(bool force = false);

  void collectTextObject(const std::shared_ptr<TextObject> &textObj, CollectedPage &page);
  void updateLinkedTexts();
  bool hasUnfinishedLinkedTexts();

  void drawLine(const std::shared_ptr<Line> &line, const CollectedPage &page);
  void drawBox(const std::shared_ptr<Box> &box, const CollectedPage &page);
  void drawRectangle(const std::shared_ptr<Box> &box, const CollectedPage &page);
  void drawOval(const std::shared_ptr<Box> &oval, const CollectedPage &page);
  void drawPolygon(const std::shared_ptr<Box> &polygon, const CollectedPage &page);
  void drawBezierBox(const std::shared_ptr<Box> &box, const CollectedPage &page);
  void drawTextBox(const std::shared_ptr<TextBox> &textbox, const CollectedPage &page);
  void drawTextPath(const std::shared_ptr<TextPath> &textPath, const CollectedPage &page);
  void drawText(const std::shared_ptr<Text> &text, const LinkedTextSettings &linkSettings);
  void drawGroup(const std::shared_ptr<Group> &group, const CollectedPage &page);

  void writeFill(librevenge::RVNGPropertyList &propList, const boost::optional<Fill> &fill);
  void writeFrame(librevenge::RVNGPropertyList &propList, const Frame &frame, const bool runaround, const bool allowHairline = false);
};

}

#endif // QXPCONTENTCOLLECTOR_H_INCLUDED

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
