//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ArrayListFieldWidget.h"

#include <algorithm>
#include <cassert>

#include "comms_champion/Property.h"

namespace comms_champion
{

ArrayListElementWidget::ArrayListElementWidget(
    FieldWidget* fieldWidget,
    QWidget* parentObj)
  : Base(parentObj)
{
    m_ui.setupUi(this);
    m_fieldWidget = fieldWidget;
    m_ui.m_layout->addWidget(fieldWidget);

    updateUi();
    connect(
        m_fieldWidget, SIGNAL(sigFieldUpdated()),
        this, SIGNAL(sigFieldUpdated()));

    connect(
        m_ui.m_removePushButton, SIGNAL(clicked()),
        this, SIGNAL(sigRemoveRequested()));

}

void ArrayListElementWidget::refresh()
{
    m_fieldWidget->refresh();
}

void ArrayListElementWidget::setEditEnabled(bool enabled)
{
    m_editEnabled = enabled;
    m_fieldWidget->setEditEnabled(enabled);
    updateUi();
}

void ArrayListElementWidget::setDeletable(bool deletable)
{
    m_deletable = deletable;
    updateUi();
}

void ArrayListElementWidget::updateProperties(const QVariantMap& props)
{
    assert(m_fieldWidget != nullptr);
    m_fieldWidget->updateProperties(props);
}

void ArrayListElementWidget::updateUi()
{
    bool deleteButtonVisible = m_editEnabled && m_deletable;
    m_ui.m_buttonWidget->setVisible(deleteButtonVisible);
    m_ui.m_sepLine->setVisible(deleteButtonVisible);
}

ArrayListFieldWidget::ArrayListFieldWidget(
    WrapperPtr wrapper,
    CreateMissingDataFieldsFunc&& updateFunc,
    QWidget* parentObj)
  : Base(parentObj),
    m_wrapper(std::move(wrapper)),
    m_createMissingDataFieldsCallback(std::move(updateFunc))
{
    m_ui.setupUi(this);
    setNameLabelWidget(m_ui.m_nameLabel);
    setValueWidget(m_ui.m_valueWidget);
    setSeparatorWidget(m_ui.m_sepLine);
    setSerialisedValueWidget(m_ui.m_serValueWidget);

    refreshInternal();
    addMissingFields();

    updateUi();

    connect(
        m_ui.m_addFieldPushButton, SIGNAL(clicked()),
        this, SLOT(addNewField()));
}

ArrayListFieldWidget::~ArrayListFieldWidget() = default;

void ArrayListFieldWidget::refreshImpl()
{
    while (!m_elements.empty()) {
        assert(m_elements.back() != nullptr);
        delete m_elements.back();
        m_elements.pop_back();
    }

    m_wrapper->refreshMembers();

    refreshInternal();
    addMissingFields();
    assert(m_elements.size() == m_wrapper->size());
}

void ArrayListFieldWidget::editEnabledUpdatedImpl()
{
    for (auto* elem : m_elements) {
        elem->setEditEnabled(isEditEnabled());
    }
    updateUi();
}

void ArrayListFieldWidget::updatePropertiesImpl(const QVariantMap& props)
{
    auto elemPropsVar = Property::getData(props);
    if (!elemPropsVar.isValid()) {
        return;
    }

    if (elemPropsVar.canConvert<QVariantMap>()) {
        updateElementsProperties(elemPropsVar.value<QVariantMap>());
        return;
    }

    if (elemPropsVar.canConvert<QVariantList>()) {
        updateElementsProperties(elemPropsVar.value<QVariantList>());
        return;
    }
}

void ArrayListFieldWidget::dataFieldUpdated()
{
    refreshInternal();
    emitFieldUpdated();
}

void ArrayListFieldWidget::addNewField()
{
    m_wrapper->addField();
    refreshImpl();
    assert(m_elements.size() == m_wrapper->size());
    emitFieldUpdated();
}

void ArrayListFieldWidget::removeField()
{
    auto* sigSender = sender();
    auto iter = std::find(m_elements.begin(), m_elements.end(), sigSender);
    if (iter == m_elements.end()) {
        assert(!"Something is not right");
        return;
    }

    auto idx = static_cast<int>(std::distance(m_elements.begin(), iter));

    m_wrapper->removeField(idx);

    refreshImpl();

    assert(m_elements.size() == m_wrapper->size());
    assert(m_elements.size() == (unsigned)m_ui.m_membersLayout->count());

    emitFieldUpdated();
}

void ArrayListFieldWidget::addDataField(FieldWidget* dataFieldWidget)
{
    auto* wrapperWidget = new ArrayListElementWidget(dataFieldWidget);
    wrapperWidget->setEditEnabled(isEditEnabled());
    wrapperWidget->setDeletable(!m_wrapper->hasFixedSize());

    if (!m_elemProperties.empty()) {
        auto elemPropsIdx = m_elements.size() % m_elemProperties.size();
        assert(elemPropsIdx < m_elemProperties.size());
        auto& elemProps = m_elemProperties[elemPropsIdx];
        wrapperWidget->updateProperties(elemProps);
    }

    connect(
        wrapperWidget, SIGNAL(sigFieldUpdated()),
        this, SLOT(dataFieldUpdated()));

    connect(
        wrapperWidget, SIGNAL(sigRemoveRequested()),
        this, SLOT(removeField()));

    m_elements.push_back(wrapperWidget);
    m_ui.m_membersLayout->addWidget(wrapperWidget);
}

void ArrayListFieldWidget::refreshInternal()
{
    QString serValueStr;
    auto serValue = m_wrapper->getSerialisedValue();

    for (auto byte : serValue) {
        if (!serValueStr.isEmpty()) {
            serValueStr.append(QChar(' '));
        }
        serValueStr.append(QString("%1").arg(byte, 2, 16, QChar('0')));
    }

    assert(m_ui.m_serValuePlainTextEdit != nullptr);
    m_ui.m_serValuePlainTextEdit->setPlainText(serValueStr);


    bool valid = m_wrapper->valid();
    setValidityStyleSheet(*m_ui.m_nameLabel, valid);
    setValidityStyleSheet(*m_ui.m_serFrontLabel, valid);
    setValidityStyleSheet(*m_ui.m_serValuePlainTextEdit, valid);
    setValidityStyleSheet(*m_ui.m_serBackLabel, valid);
}

void ArrayListFieldWidget::updateUi()
{
    bool addButtonVisible = isEditEnabled() && (!m_wrapper->hasFixedSize());
    m_ui.m_addSepLine->setVisible(addButtonVisible);
    m_ui.m_addFieldPushButton->setVisible(addButtonVisible);
}

void ArrayListFieldWidget::addMissingFields()
{
    if (!m_createMissingDataFieldsCallback) {
        assert(!"Callback should exist");
        return;
    }

    assert(m_elements.empty());
    auto fieldWidgets = m_createMissingDataFieldsCallback(*m_wrapper);
    for (auto& fieldWidgetPtr : fieldWidgets) {
        addDataField(fieldWidgetPtr.release());
    }

    assert(m_elements.size() == m_wrapper->size());
    assert(m_elements.size() == (unsigned)m_ui.m_membersLayout->count());
}

void ArrayListFieldWidget::updateElementsProperties(const QVariantMap& props)
{
    m_elemProperties.clear();
    m_elemProperties.push_back(props);

    for (auto* elem : m_elements) {
        elem->updateProperties(props);
    }
}

void ArrayListFieldWidget::updateElementsProperties(const QVariantList& propsList)
{
    decltype(m_elemProperties) props;
    for (auto idx = 0; idx < propsList.size(); ++idx) {
        auto& elemPropsVar = propsList[idx];
        if ((!elemPropsVar.isValid()) || (!elemPropsVar.canConvert<QVariantMap>())) {
            return;
        }

        props.push_back(elemPropsVar.value<QVariantMap>());
    }

    m_elemProperties.swap(props);
    if (m_elemProperties.empty()) {
        return;
    }

    auto propIdx = 0U;
    for (auto* elemWidget : m_elements) {
        auto& elemProps = m_elemProperties[propIdx];
        elemWidget->updateProperties(elemProps);

        ++propIdx;
        if (m_elemProperties.size() <= propIdx) {
            propIdx = 0U;
        }
    }
}


}  // namespace comms_champion


