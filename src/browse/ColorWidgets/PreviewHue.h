#ifndef PREVIEW_HUE_H
#define PREVIEW_HUE_H

#include <QDialog>
#include <QPushButton>
#include "color_preview.hpp"
#include "hue_slider.hpp"

class PreviewHue : public color_widgets::ColorPreview
{
  Q_OBJECT

public:
  explicit PreviewHue(QWidget *parent = 0);

private slots:
  void clickedButton();

};


#endif
