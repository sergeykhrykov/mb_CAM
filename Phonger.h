// hopfer.benjamin@gmail.com
// http://benjaminhopfer.com
//
// Phonger example application for Qt / vtk
// 32bit/64bit cross compile

#ifndef Phonger_H
#define Phonger_H

#include <vector>

#include <vtkActor.h>
#include "vtkBoxWidget.h"
#include <vtkCommand.h>
#include "vtkCubeSource.h"
#include <vtkLinearSubdivisionFilter.h>
#include "vtkLinearTransform.h"
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include "vtkSTLWriter.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QtWidgets/QListWidget>


class Ui_Phonger;

class Phonger : public QMainWindow {
  Q_OBJECT

 public:
  Phonger();

  ~Phonger(){};

 private slots:

  void slotOpenStl();

  void slotSetAmbientColor();

  void slotSetDiffuseColor();

  void slotSetSpecularColor();

  void slotDsbChanged(double newPower);

  void slotExit();

  void slotAddSupport();

  void slotSaveSupport();

  void slotCurrentSupportChanged(QListWidgetItem *current,
                                 QListWidgetItem *previous);

 private:
  // Designer form
  Ui_Phonger *ui;

  QString _lastOpenedPath;
  vtkSmartPointer<vtkSTLReader> _stlReader;
  vtkSmartPointer<vtkPolyDataMapper> _mapper;
  vtkSmartPointer<vtkPolyDataNormals> _normals;
  vtkSmartPointer<vtkLinearSubdivisionFilter> _subdiv;
  vtkSmartPointer<vtkActor> _actor;
  vtkSmartPointer<vtkRenderer> _renderer;

  std::vector<vtkSmartPointer<vtkActor>> m_actors;
  std::vector<vtkSmartPointer<vtkPolyDataMapper>> m_mappers;
  std::map<int, vtkSmartPointer<vtkBoxWidget>> m_widgets;

  QMap<QString, int> m_SupportNamesIndices;

  void loadFile(QString const &filePath);

  void pickColor(double const *curColor, QString const &descr,
                 double *newColor);

  void prop2Gui();

  void gui2Prop();

  void update3d();
};

class WidgetCallback : public vtkCommand {
 public:
  static WidgetCallback *New() { return new WidgetCallback; }
  virtual void Execute(vtkObject *caller, unsigned long, void *);
};

#endif