#include "PreviewHue.h"
#include "CustomLogger.h"
#include <QBoxLayout>
#include <QKeyEvent>

PreviewHue::PreviewHue(QWidget *parent)
  : color_widgets::ColorPreview(parent)
{
  connect(this, SIGNAL(clicked()), this, SLOT(clickedButton()));
}

void PreviewHue::clickedButton()
{
  QDialog *popup = new QDialog(this);
  QBoxLayout *popupLayout = new QHBoxLayout(popup);
  popupLayout->setMargin(2);

  color_widgets::HueSlider* slider_
      = new color_widgets::HueSlider(Qt::Horizontal, popup);
  slider_->setColor(color());

  popupLayout->addWidget(slider_);

  popup->move(mapToGlobal(rect().topLeft()));
  popup->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

  popup->exec();
  setColor(slider_->color());
  delete popup;

  emit colorChanged(color());
}
