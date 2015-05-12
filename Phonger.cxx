// hopfer.benjamin@gmail.com
// http://benjaminhopfer.com
// 
// Phonger example application for Qt / vtk
// 32bit/64bit cross compile

#include "Phonger.h"

#include "ui_Phonger.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QStandardPaths>

#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>



static double const GRADIENT_BACKGROUND_TOP[3] = { 0.4, 0.4, 0.4 };
static double const GRADIENT_BACKGROUND_BOT[3] = { 0.6, 0.6, 0.6 };

 
Phonger::Phonger()
: _lastOpenedPath (R"(C:\Users\1\SkyDrive\bmstu\medical\_Models\2015.05_Pegashev\Fixtures)"),
  _stlReader(vtkSmartPointer<vtkSTLReader>::New()),
  _mapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
  _normals(vtkSmartPointer<vtkPolyDataNormals>::New()),
  _subdiv(vtkSmartPointer<vtkLinearSubdivisionFilter>::New()),
  _actor(vtkSmartPointer<vtkActor>::New()),
  _renderer(vtkSmartPointer<vtkRenderer>::New())
{
  this->ui = new Ui_Phonger;
  this->ui->setupUi(this);
 
  connect(this->ui->pbOpenSTL, SIGNAL(clicked()), this, SLOT(slotOpenStl()));

  connect(this->ui->pbColAmbient, SIGNAL(clicked()), this, SLOT(slotSetAmbientColor()));
  connect(this->ui->pbColDiffuse, SIGNAL(clicked()), this, SLOT(slotSetDiffuseColor()));
  connect(this->ui->pbColSpecular, SIGNAL(clicked()), this, SLOT(slotSetSpecularColor()));

  connect(this->ui->AddSupportButton, SIGNAL(clicked()), this, SLOT(slotAddSupport()));
  connect(this->ui->SaveSupportButton, SIGNAL(clicked()), this, SLOT(slotSaveSupport()));
  connect(this->ui->OpenFrameButton, SIGNAL(clicked()), this, SLOT(slotOpenFrame()));

  connect(
      this->ui->SupportsList,
      SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this,
      SLOT(slotCurrentSupportChanged(QListWidgetItem *, QListWidgetItem *)));

  connect(this->ui->dsbAmbCoeff, SIGNAL(valueChanged(double)), this, SLOT(slotDsbChanged(double)));
  connect(this->ui->dsbDiffCoeff, SIGNAL(valueChanged(double)), this, SLOT(slotDsbChanged(double)));
  connect(this->ui->dsbSpecCoeff, SIGNAL(valueChanged(double)), this, SLOT(slotDsbChanged(double)));
  connect(this->ui->dsbSpecPower, SIGNAL(valueChanged(double)), this, SLOT(slotDsbChanged(double)));

  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

  prop2Gui();

  update3d();
}
 

void Phonger::loadFile(QString const &filePath)
{
  _stlReader->SetFileName(filePath.toLocal8Bit());
  _stlReader->SetMerging(1);

  _normals->SetInputConnection(_stlReader->GetOutputPort());
  _normals->ConsistencyOn();
  _normals->SplittingOn();
  _normals->SetFeatureAngle(30);

  _normals->Update();

  _mapper->SetInputConnection(_normals->GetOutputPort());

  _actor->SetMapper(_mapper);
  _actor->GetProperty()->SetRepresentationToSurface();
  _actor->GetProperty()->SetInterpolationToPhong();

  // VTK Renderer
  _renderer = vtkSmartPointer<vtkRenderer>::New();
  _renderer->GradientBackgroundOn();
  _renderer->SetBackground(GRADIENT_BACKGROUND_TOP[0], GRADIENT_BACKGROUND_TOP[1], GRADIENT_BACKGROUND_TOP[2]);
  _renderer->SetBackground2(GRADIENT_BACKGROUND_BOT[0], GRADIENT_BACKGROUND_BOT[1], GRADIENT_BACKGROUND_BOT[2]);
  _renderer->GetActiveCamera()->ParallelProjectionOn();

  // Transparency support
  vtkRenderWindow *renderWindow = this->ui->qvtkWidget->GetRenderWindow();
  renderWindow->SetAlphaBitPlanes(1);
  renderWindow->SetMultiSamples(0);

  _renderer->SetUseDepthPeeling(1);
  _renderer->SetMaximumNumberOfPeels(10);
  _renderer->SetOcclusionRatio(0.05);

  // VTK/Qt wedded
  this->ui->qvtkWidget->GetRenderWindow()->RemoveRenderer(_renderer);
  this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(_renderer);

  update3d();
}


void Phonger::prop2Gui()
{
  ui->dsbAmbCoeff->setValue(_actor->GetProperty()->GetAmbient());
  ui->dsbDiffCoeff->setValue(_actor->GetProperty()->GetDiffuse());
  ui->dsbSpecCoeff->setValue(_actor->GetProperty()->GetSpecular());
  ui->dsbSpecPower->setValue(_actor->GetProperty()->GetSpecularPower());

}


void Phonger::gui2Prop()
{
  _actor->GetProperty()->SetAmbient(ui->dsbAmbCoeff->value());
  _actor->GetProperty()->SetDiffuse(ui->dsbDiffCoeff->value());
  _actor->GetProperty()->SetSpecular(ui->dsbSpecCoeff->value());
  _actor->GetProperty()->SetSpecularPower(ui->dsbSpecPower->value());

  update3d();
}


void Phonger::update3d()
{
  _renderer->RemoveActor(_actor);
  _renderer->AddActor(_actor);

  ui->qvtkWidget->update();
}


void Phonger::slotOpenStl()
{
  QStringList folders;
  folders.push_back(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0]);
  folders.push_back(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0]);
  folders.push_back(QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]);

  QFileDialog fileDialog(this, tr("Choose Stl file"));
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setDirectory(_lastOpenedPath);
  fileDialog.setHistory(folders);
  fileDialog.setNameFilters({ tr("Stl files (*.stl)") });
  fileDialog.setViewMode(QFileDialog::Detail);


  if (fileDialog.exec())
  {
    QString firstPath = fileDialog.selectedFiles()[0];
    _lastOpenedPath = firstPath;
    loadFile(firstPath);
  }

  _renderer->RemoveActor(_actor);
  _renderer->AddActor(_actor);

  _renderer->ResetCameraClippingRange();
  _renderer->ResetCamera();
}


void Phonger::slotSetAmbientColor()
{
  double curColor[3], newColor[3];

  _actor->GetProperty()->GetAmbientColor(curColor);

  pickColor(curColor, tr("ambient"), newColor);

  _actor->GetProperty()->SetAmbientColor(newColor);

  update3d();
}


void Phonger::slotSetDiffuseColor()
{
  double curColor[3], newColor[3];

  _actor->GetProperty()->GetDiffuseColor(curColor);

  pickColor(curColor, tr("diffuse"), newColor);

  _actor->GetProperty()->SetDiffuseColor(newColor);

  update3d();
}


void Phonger::slotSetSpecularColor()
{
  double curColor[3], newColor[3];

  _actor->GetProperty()->GetSpecularColor(curColor);

  pickColor(curColor, tr("specular"), newColor);

  _actor->GetProperty()->SetSpecularColor(newColor);

  update3d();
}


void Phonger::pickColor(double const *curColor, QString const &descr, double *newColor)
{
  QColor curQColor(curColor[0] * 255, curColor[1] * 255, curColor[2] * 255);

  QColor pickedColor = QColorDialog::getColor(curQColor, this, tr("Pick %1 color").arg(descr));

  newColor[0] = pickedColor.redF();
  newColor[1] = pickedColor.greenF();
  newColor[2] = pickedColor.blueF();
}

void Phonger::slotDsbChanged(double newPower) { gui2Prop(); }

void Phonger::slotExit() { qApp->exit(); }

void Phonger::slotAddSupport() {
  auto support_source = vtkSmartPointer<vtkCubeSource>::New();

  auto transform = vtkSmartPointer<vtkTransform>::New();
  transform->Scale(5, 2, 2);

  auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
  transformFilter->SetInputConnection(support_source->GetOutputPort());
  transformFilter->SetTransform(transform);

  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(transformFilter->GetOutputPort());
  auto actor = vtkSmartPointer<vtkActor>::New();
  actor->GetProperty()->SetColor(0.8, 0.1, 0.1);

  actor->SetMapper(mapper);

  // Populate mappers and actors with newly created for the support
  m_mappers.push_back(mapper);
  m_actors.push_back(actor);

  // Add data to the supports list in GUI and remember its index for identifying
  // correct actor/mapper in future
  QString support_name("Support ");
  int support_number = m_actors.size();
  support_name.append(QString::number(support_number));

  m_SupportNamesIndices[support_name] =
      support_number - 1;  // Corresponds to the actual index in vector

  this->ui->SupportsList->addItem(support_name);

  if (_renderer == nullptr) {
    _renderer = vtkSmartPointer<vtkRenderer>::New();
  }
  _renderer->GradientBackgroundOn();
  _renderer->SetBackground(GRADIENT_BACKGROUND_TOP[0],
                           GRADIENT_BACKGROUND_TOP[1],
                           GRADIENT_BACKGROUND_TOP[2]);
  _renderer->SetBackground2(GRADIENT_BACKGROUND_BOT[0],
                            GRADIENT_BACKGROUND_BOT[1],
                            GRADIENT_BACKGROUND_BOT[2]);

  _renderer->GetActiveCamera()->ParallelProjectionOff();

  // Transparency support
  vtkRenderWindow *renderWindow = this->ui->qvtkWidget->GetRenderWindow();
  renderWindow->SetAlphaBitPlanes(1);
  renderWindow->SetMultiSamples(0);

  _renderer->SetUseDepthPeeling(1);
  _renderer->SetMaximumNumberOfPeels(10);
  _renderer->SetOcclusionRatio(0.05);

  // VTK/Qt wedded
  this->ui->qvtkWidget->GetRenderWindow()->RemoveRenderer(_renderer);
  this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(_renderer);

  for (auto a : m_actors) {
    _renderer->RemoveActor(a);
    _renderer->AddActor(a);
  }

  _renderer->ResetCameraClippingRange();
  //_renderer->
  ui->qvtkWidget->update();
  //_renderer->ResetCamera();
}

void Phonger::slotSaveSupport() {
  QListWidget *list = this->ui->SupportsList;
  auto current_item = list->currentItem();
  if (current_item == nullptr) {
    QMessageBox msg_box;
    msg_box.setText("No Support is selected.");
    msg_box.setInformativeText(
        R"(Please select a support from the list or use "Add Support" button to create one and then select it.)");
    msg_box.exec();
    return;
  }
  int current_item_index = m_SupportNamesIndices[current_item->text()];

  auto widget = m_widgets.at(current_item_index); 

  auto t = vtkSmartPointer<vtkTransform>::New();
  widget->GetTransform(t);

  auto mapper = m_mappers.at(current_item_index);

  auto triangle_filter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInputData(mapper->GetInput());
  triangle_filter->Update();

  auto t_pd_f = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  t_pd_f->SetTransform(t);
  t_pd_f->SetInputData(triangle_filter->GetOutput());
  t_pd_f->Update();
  
  std::string filename = current_item->text().toStdString() + ".stl";

  auto stl_writer = vtkSmartPointer<vtkSTLWriter>::New();  
  stl_writer->SetFileName(filename.c_str());
  stl_writer->SetInputData(t_pd_f->GetOutput());
  stl_writer->Write();

  triangle_filter->RemoveAllInputs();
  stl_writer->RemoveAllInputs();
}

void WidgetCallback::Execute(vtkObject *caller, unsigned long, void *) {
  auto t = vtkSmartPointer<vtkTransform>::New();
  auto widget = static_cast<vtkBoxWidget *>(caller);
  widget->GetTransform(t);
  widget->GetProp3D()->SetUserTransform(t);
}

void Phonger::slotCurrentSupportChanged(QListWidgetItem * current, QListWidgetItem * previous)
{
  int curr_support_index = m_SupportNamesIndices[current->text()];

  // Disable (hide) previously active widget is there was one
  if (previous != nullptr)
  {
	  int prev_support_index = m_SupportNamesIndices[previous->text()];
	  auto prev_widget = m_widgets.at(prev_support_index);
	  prev_widget->Off();
  }

  vtkSmartPointer<vtkBoxWidget> widget = nullptr;

  if (m_widgets.count(curr_support_index) ==
      0)  // Check if a widget for current support has been created already
  {
    // If not, create a new widget
	  widget = vtkSmartPointer<vtkBoxWidget>::New();
	  m_widgets[curr_support_index] = widget;
	  
    // And set it up
    auto iren = this->ui->qvtkWidget->GetInteractor();
	widget->SetInteractor(iren);
	widget->SetPlaceFactor(1.25);
	widget->SetProp3D(m_actors[curr_support_index]);

	widget->PlaceWidget();
    auto callback = vtkSmartPointer<WidgetCallback>::New();
    widget->AddObserver(vtkCommand::InteractionEvent, callback);
  }
  else // Widget already exists
  {
	  widget = m_widgets[curr_support_index];
  }

  widget->On();

  _renderer->ResetCameraClippingRange();
  this->ui->qvtkWidget->update();
}

void Phonger::slotOpenFrame()
{
	QStringList folders;
	folders.push_back(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0]);
	folders.push_back(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0]);
	folders.push_back(QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]);

	QFileDialog fileDialog(this, tr("Choose STL file with Frame"));
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	fileDialog.setDirectory(_lastOpenedPath);
	fileDialog.setHistory(folders);
	fileDialog.setNameFilters({ tr("STL files (*.stl)") });
	fileDialog.setViewMode(QFileDialog::Detail);
	
	if ( (m_SupportNamesIndices.count("Frame")) > 0 )
	{
		QMessageBox msg_box;
		msg_box.setText("Frame is already loaded. It will be replaced.");		
		msg_box.exec();
		auto frame_index = m_SupportNamesIndices["Frame"];
		m_SupportNamesIndices.remove("Frame");
		
		m_actors[frame_index] = nullptr;
		m_mappers[frame_index] = nullptr;
		
		if (m_widgets.count(frame_index) > 0){
			m_widgets[frame_index] = nullptr;
		}
		// Not finished with this
	}

    if (!fileDialog.exec()) { // No file was chosen
        return;
    }

    QString file_path = fileDialog.selectedFiles()[0];
    _lastOpenedPath = file_path;

	auto stl_reader = vtkSmartPointer<vtkSTLReader>::New();
	
	stl_reader->SetFileName(file_path.toLocal8Bit());
	stl_reader->SetMerging(1);
	stl_reader->Update();
	    
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(stl_reader->GetOutputPort());
    
	auto actor = vtkSmartPointer<vtkActor>::New();
    actor->GetProperty()->SetColor(0.1, 0.8, 0.1);
	actor->GetProperty()->SetOpacity(.5);
    actor->SetMapper(mapper);

    // Populate mappers and actors with newly created for the support
    m_mappers.push_back(mapper);
    m_actors.push_back(actor);

    // Add data to the supports list in GUI and remember its index for
    // identifying
    // correct actor/mapper in future
    QString support_name("Frame");
    int support_number = m_actors.size();    

    m_SupportNamesIndices[support_name] =
        support_number - 1;  // Corresponds to the actual index in vector

    this->ui->SupportsList->addItem(support_name);

	_renderer->RemoveActor(actor);
	_renderer->AddActor(actor);

    _renderer->ResetCameraClippingRange();
	_renderer->ResetCamera();
}