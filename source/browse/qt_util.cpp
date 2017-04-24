#include "qt_util.h"
#include <stdlib.h>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>
#include <QLayout>
#include <QLayoutItem>
#include "CustomLogger.h"

QString CustomSaveFileDialog(QWidget *parent,
                             const QString &title,
                             const QString &directory,
                             const QString &filter)
{

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
  return QFileDialog::getSaveFileName(parent,
                                      title,
                                      directory,
                                      filter);
#else
  QFileDialog dialog(parent, title, directory, filter);
  if (parent)
    dialog.setWindowModality(Qt::WindowModal);

  QRegExp filter_regex(QLatin1String("(?:^\\*\\.(?!.*\\()|\\(\\*\\.)(\\w+)"));
  QStringList filters = filter.split(QLatin1String(";;"));
  if (!filters.isEmpty())
  {
    dialog.setNameFilter(filters.first());
    if (filter_regex.indexIn(filters.first()) != -1)
      dialog.setDefaultSuffix(filter_regex.cap(1));
  }
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() == QDialog::Accepted)
  {
    QString file_name = dialog.selectedFiles().first();
    QFileInfo info(file_name);
    if (info.suffix().isEmpty() && !dialog.selectedNameFilter().isEmpty())
      if (filter_regex.indexIn(dialog.selectedNameFilter()) != -1)
      {
        QString extension = filter_regex.cap(1);
        file_name += QLatin1String(".") + extension;
      }

    //    QFile file(file_name);
    //    if (file.exists()) {
    //        QMessageBox msgBox;
    //        msgBox.setText("Replace?");
    //        msgBox.setInformativeText("File \'" + file_name + "\' already exists. Replace?");
    //        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    //        msgBox.setDefaultButton(QMessageBox::Cancel);
    //        if (msgBox.exec() != QMessageBox::Yes)
    //          return QString();
    //    }

    return file_name;
  }
  else
    return QString();
#endif  // Q_WS_MAC || Q_WS_WIN
}

bool validateFile(QWidget* parent, QString name, bool write)
{
  QFile file(name);
  if (name.isEmpty())
    return false;

  if (!write)
  {
    if (!file.exists())
    {
      QMessageBox::warning(parent, "Failed", "File does not exist.");
      return false;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
      QMessageBox::warning(parent, "Failed", "Could not open file for reading.");
      return false;
    }
  }
  else
  {
    if (file.exists() && !file.remove())
    {
      QMessageBox::warning(parent, "Failed", "Could not delete file.");
      return false;
    }
    if (!file.open(QIODevice::WriteOnly))
    {
      QMessageBox::warning(parent, "Failed", "Could not open file for writing.");
      return false;
    }
  }
  file.close();
  return true;
}


QColor generateColor()
{
  int H = rand() % 359;
//  int S = rand() % 64 + 191;
  int V = rand() % 54 + 181;
  int A = 128;
  return QColor::fromHsv(H, 255, V, A);
}

QString catExtensions(std::list<std::string> exts)
{
  QString ret;
  for (auto &p : exts)
  {
    ret += "*." + QString::fromStdString(p);
    if (p != exts.back())
      ret += " ";
  }
  return ret;
}

QString catFileTypes(QStringList types)
{
  QString ret;
  for (auto &q : types)
  {
    if (q != types.front())
      ret += ";;";
    ret += q;
  }
  return ret;
}

void add_to_table(QTableWidget *table,
                  int row, int col, std::string data,
                  QVariant value, QBrush background)
{
  QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(data));
  //  item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  item->setFlags(item->flags() ^ Qt::ItemIsEditable);
  item->setData(Qt::UserRole, value);
  item->setData(Qt::BackgroundRole, background);
  table->setItem(row, col, item);
  //  DBG << "added " << data << " and " << value.toDouble();
}

QString path_of_file(QString filename)
{
  QString ret;
  QFileInfo file(filename);
  if (file.absoluteDir().isReadable())
    ret = file.absoluteDir().absolutePath();
  return ret;
}

void clearLayout(QLayout* layout, bool deleteWidgets)
{
  QLayoutItem* item;
  while (layout->count() && (item = layout->takeAt(0)))
  {
    QWidget* widget;
    if ( (deleteWidgets) && (widget = item->widget()))
      delete widget;
    if (QLayout* childLayout = item->layout())
      clearLayout(childLayout, deleteWidgets);
    delete item;
  }
}
