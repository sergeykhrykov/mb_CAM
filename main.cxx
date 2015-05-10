// hopfer.benjamin@gmail.com
// http://benjaminhopfer.com
// 
// Phonger example application for Qt / vtk4
// 32bit/64bit cross compile

#include <QApplication>
#include "Phonger.h"
 
int main( int argc, char** argv )
{
  // QT Stuff
  QApplication app( argc, argv );
 
  Phonger phonger;
  phonger.show();
 
  return app.exec();
}
