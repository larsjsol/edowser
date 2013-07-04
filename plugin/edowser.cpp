#include "edowser.h"
#include "xmouse.h"

#include <QApplication>
#include <QDebug>
#include <QX11EmbedContainer>
#include <QTimer>
#include <QTemporaryFile>


Edowser::Edowser(QWidget *parent)
  : QWidget(parent), QtNPBindable() {
  qDebug() << "PID:" << QCoreApplication::applicationPid();

  //we cant get focus the normal way, so we use manymouse/xlib to get mousevents
  xmouse = Xmouse::subscribe();
  connect(xmouse, SIGNAL(press(int, int, int)), this, SLOT(mouse_press(int, int, int)));

  windowActive = true;
  tabVisible = true;

  //create a container-widget
  container = new QX11EmbedContainer(this);
  qDebug() << "XID:" << container->winId();

  qDebug() << "parameters:";
  QMap<QByteArray, QVariant> html_parameters = parameters();
  QMap<QByteArray, QVariant>::const_iterator i = parameters().constBegin();
  while (i != parameters().constEnd()) {
    qDebug() << i.key() << "=" << i.value();
    ++i;
  }                       

  //get a name for a temp file
  tmpfile = new QTemporaryFile(QDir::tempPath() + "/" + "edowser_XXXXXX.txt");
  tmpfile->open(); 
  if (html_parameters.contains("originaltext"))
    tmpfile->write(html_parameters["originaltext"].toString().toUtf8());
  tmpfile->close();

  //start an editor that embeds itself into our widget
  //QString command("xterm  -into %x -e \"/usr/bin/vi %f\"");
  QString command("emacs --parent-id %x %f");
  //QString command("gvim --socketid %x %f");
  format(&command, tmpfile->fileName(), QString::number(container->winId()));
  qDebug() << "editor command:" << command;
               
  process = new QProcess(this);
  process->start(command);
  qDebug() << "editor PID:" << process->pid();
  process->waitForStarted();

  //emacs ignores resizeevents until after it is done initializing
  for (int i = 1000; i <= 10000; i+= 1000)
    QTimer::singleShot(i, this, SLOT(dummy_resize()));
}

Edowser::~Edowser(){
  xmouse->unsubscribe(); //xmouse deletes itself
  process->terminate();
  process->waitForFinished();
  delete container;
  delete process;
  delete tmpfile;

}

QString Edowser::text() const {
  QFile file(tmpfile->fileName());
  file.open(QIODevice::ReadOnly);
  QString result(file.readAll());
  file.close();
  return result;
}

//it appears that focus is broken in qtbrowserplugin
//we grab the keyboard on mouse clicks inside of the editor
void Edowser::mouse_press(int x, int y, int button) {
  if (button == 0) { //first (left) mouse button

    QPoint lpoint = mapFromGlobal(QPoint(x, y));
    if (geometry().contains(lpoint) && windowActive && tabVisible) {
      container->grabKeyboard();
    } else {
      container->releaseKeyboard();
    }
  }
}

void Edowser::mouse_release(int x, int y, int button) {
  (void)x;
  (void)y;
  (void)button;
}

void Edowser::dummy_resize() {
  int w = width();
  int h = height();
  resize(w + 1, h);
  resize(w, h);
}

void Edowser::setText(const QString &text) {
  QFile file(tmpfile->fileName());
  file.open(QIODevice::WriteOnly);
  file.write(text.toUtf8()); // um...
  file.close();
}

void Edowser::pageFocus() {
  tabVisible = true;
}

void Edowser::pageBlur() {
  tabVisible = false;
  container->releaseKeyboard();
}

void Edowser::resizeEvent(QResizeEvent *event) {
  //qDebug() << "resizeEvent()" << event->size().width() << "x" << event->size().height();
  container->setGeometry(0, 0, event->size().width(), event->size().height());
}

bool Edowser::event(QEvent *event) {
  //qDebug() << "event():" << event;
  switch (event->type()) {
  case QEvent::WindowActivate:
    windowActive = true;
    return true;
  case QEvent::WindowDeactivate:
    windowActive = false;
    container->releaseKeyboard();
    return true;
  default:
    return QWidget::event(event);
    
  }
}


void Edowser::format(QString *frm_str, const QString &filename, const QString &xid) {
  frm_str->replace("%f", filename);
  frm_str->replace("%x", xid);
}


int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  Edowser *e = new Edowser();
  e->resize(500, 300);
  e->setWindowTitle("Edowser");
  e->show();

  int retval = app.exec();
  delete e;

  return retval;
}

QTNPFACTORY_BEGIN("Edowser", "A plugin that lets you edit textareas with a proper editor.")
QTNPCLASS(Edowser)
QTNPFACTORY_END()
