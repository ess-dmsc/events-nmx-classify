#include <QSettings>
#include <QRegExp>
#include "SearchBox.h"
#include "ui_SearchBox.h"

SearchBox::SearchBox(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::SearchBox)
{
  ui->setupUi(this);
}

SearchBox::~SearchBox()
{
  delete ui;
}

void SearchBox::setList(QStringList lst)
{
  set_ = lst;
  on_lineFilter_textChanged(ui->lineFilter->text());
}

QStringList SearchBox::selection() const
{
  return selected_set_;
}

void SearchBox::on_lineFilter_textChanged(const QString &arg1)
{
  QRegExp regExp(arg1, Qt::CaseInsensitive, QRegExp::Wildcard);
  selected_set_ = set_.filter(regExp);
  ui->listSubset->clear();
  ui->listSubset->addItems(selected_set_);

  emit selectionChanged();
}
