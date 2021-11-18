#include "pclvisualizer.h"

#include "./ui_pclvisualizer.h"
#include <QColor>
#include <QColorDialog>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QPainter>
#include <QTextList>
#include <QTextStream>
PCLVisualizer::PCLVisualizer(QWidget* parent)
  : QMainWindow(parent)
  , point_size(1)
  , ui(new Ui::PCLVisualizer)
  , bgColor(60, 80, 100)

{
  ui->setupUi(this);

  QString str = "PointCloudViewer";
  this->setWindowTitle(str);
  logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
           "] " + "[MainWindow] " + "System initializing";
  ui->logList->addItem(logStr);

  //  //创建动作，工具栏以及菜单栏
  createActions();
  //    createMenus();
  createToolBars();

  initPointCloud();
  // 给QVTK配置PCLViewer显示
  viewer_.reset(new pcl::visualization::PCLVisualizer("viewer", false));
  //设置背景颜色
  viewer_->setBackgroundColor(double(bgColor.red()) / 255,
                              double(bgColor.green()) / 255,
                              double(bgColor.blue()) / 255);

  ui->qvtkWidget->SetRenderWindow(viewer_->getRenderWindow());
  viewer_->setupInteractor(ui->qvtkWidget->GetInteractor(),
                           ui->qvtkWidget->GetRenderWindow());
  ui->qvtkWidget->update();
  connectSS();

  // Color the randomly generated cloud
  // 以随机颜色填充点云
  colorCloudDistances();

  viewer_->addPointCloud(cloud_, "cloud");
  viewer_->setPointCloudRenderingProperties(
    pcl::visualization::PCL_VISUALIZER_POINT_SIZE, point_size);
  // viewer_->addCoordinateSystem(1);
  viewer_->resetCamera();
  viewer_->setLookUpTableID("cloud");
  ui->qvtkWidget->update();

  logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
           "] " + "[MainWindow] " + "System initialize done";
  ui->logList->addItem(logStr);
}

PCLVisualizer::~PCLVisualizer()
{
  delete ui;
}

void
PCLVisualizer::createActions()
{
  //添加点云
  addCloudAction =
    new QAction(QIcon(":/images/files/cloud.png"), "添加点云", this);
  addCloudAction->setShortcut(tr("Ctrl+O"));    //(b)
  addCloudAction->setStatusTip(tr("添加点云")); //(c)

  //新建工作台
  newWorkStationAction =
    new QAction(QIcon(":/images/files/add.png"), tr("new"), this);
  newWorkStationAction->setShortcut(tr("Ctrl+N"));
  newWorkStationAction->setStatusTip(tr("new"));

  //打开点云文件
  newCloudAction =
    new QAction(QIcon(":/images/files/new2.png"), tr("打开点云文件"), this);
  //  exitAction->setShortcut(tr("Ctrl+Q"));
  newCloudAction->setStatusTip(tr("打开点云文件"));

  //复制点云
  copyCloudAction =
    new QAction(QIcon(":/images/files/copy.png"), tr("复制点云"), this);
  copyCloudAction->setShortcut(tr("Ctrl+C"));
  copyCloudAction->setStatusTip(tr("复制点云"));

  //剪切点云
  cutCloudAction =
    new QAction(QIcon(":/images/files/cut.png"), tr("剪切点云"), this);
  cutCloudAction->setShortcut(tr("Ctrl+X"));
  cutCloudAction->setStatusTip(tr("剪切点云"));

  //粘贴点云
  pasteCloudAction =
    new QAction(QIcon(":/images/files/paste.png"), tr("粘贴点云"), this);
  pasteCloudAction->setShortcut(tr("Ctrl+V"));
  pasteCloudAction->setStatusTip(tr("粘贴点云"));

  //查找点云文件
  searchCloudAction =
    new QAction(QIcon(":/images/files/search.png"), tr("查找点云文件"), this);
  searchCloudAction->setShortcut(tr("Ctrl+F"));
  searchCloudAction->setStatusTip(tr("查找点云文件"));

  //导出点云至PCD文件
  exportCloud2PCDAction = new QAction(
    QIcon(":/images/files/pointCloud.png"), tr("导出点云至PCD文件"), this);
  exportCloud2PCDAction->setStatusTip(tr("导出点云至PCD文件"));

  //导出点云至PLY文件
  exportCloud2PLYAction = new QAction(
    QIcon(":/images/files/cloud2.png"), tr("导出点云至PLY文件"), this);
  exportCloud2PLYAction->setStatusTip(tr("导出点云至PLY文件"));

  //导出点云至CSV文件
  export2CSVAction =
    new QAction(QIcon(":/images/files/CSV.png"), tr("导出点云至CSV文件"), this);
  export2CSVAction->setStatusTip(tr("导出点云至CSV文件"));

  //导出点云至TXT文件
  export2TXTAction =
    new QAction(QIcon(":/images/files/txt.png"), tr("导出点云至TXT文件"), this);
  export2TXTAction->setStatusTip(tr("导出点云至TXT文件"));

  //收藏点云文件
  starCloudAction =
    new QAction(QIcon(":/images/files/star.png"), tr("收藏点云文件"), this);
  starCloudAction->setStatusTip(tr("收藏点云文件"));

  //导出点云处理日志
  exportLogAction =
    new QAction(QIcon(":/images/files/log.png"), tr("导出点云处理日志"), this);
  exportLogAction->setStatusTip(tr("导出点云处理日志"));

  //导出屏幕截图
  snapShotAction =
    new QAction(QIcon(":/images/files/snapshot.png"), tr("导出屏幕截图"), this);
  snapShotAction->setStatusTip(tr("导出屏幕截图"));

  //离群点移除
  outliersRemoveAction = new QAction(
    QIcon(":/images/algorithm/KMeans.png"), tr("outliersRemove"), this);
  outliersRemoveAction->setStatusTip(tr("outliersRemove"));

  //滤波平滑
  filterAction =
    new QAction(QIcon(":/images/algorithm/filter.png"), tr("滤波平滑"), this);
  filterAction->setStatusTip(tr("滤波平滑"));

  //点云下采样
  downSampleAction =
    new QAction(QIcon(":/images/algorithm/density.png"), "downSampling", this);
  downSampleAction->setStatusTip(tr("downSampling"));

  //点云拼接
  cloudSpliceAction =
    new QAction(QIcon(":/images/algorithm/pingjie.png"), "点云拼接", this);
  cloudSpliceAction->setStatusTip(tr("点云拼接"));

  //点云直方图
  HistogramAction =
    new QAction(QIcon(":/images/algorithm/Histogram.png"), "Histogram", this);
  HistogramAction->setStatusTip(tr("Histogram"));

  //表面重建
  surfaceAction =
    new QAction(QIcon(":/images/algorithm/matrix.png"), "surface", this);
  surfaceAction->setStatusTip(tr("surface"));

  //点云配准
  alignAction =
    new QAction(QIcon(":/images/algorithm/DBSCAN.png"), "点云配准", this);
  alignAction->setStatusTip(tr("点云配准"));
  // MLS细化
  MLSAction =
    new QAction(QIcon(":/images/algorithm/nihe.png"), "MLS细化", this);
  MLSAction->setStatusTip(tr("MLS细化"));
}

void
PCLVisualizer::createMenus()
{
  //文件管理菜单
  fileMenu = menuBar()->addMenu(tr("测试")); //(a)
  fileMenu->addAction(addCloudAction);       //(b)
  fileMenu->addAction(newWorkStationAction);
  fileMenu->addAction(exportCloud2PCDAction);
  fileMenu->addAction(exportCloud2PLYAction);
  fileMenu->addSeparator();
  fileMenu->addAction(newCloudAction);
  //缂╂斁鑿滃崟
  zoomMenu = menuBar()->addMenu(tr("测试"));
  zoomMenu->addAction(copyCloudAction);
  zoomMenu->addAction(cutCloudAction);
  zoomMenu->addAction(pasteCloudAction);
  zoomMenu->addAction(searchCloudAction);
  zoomMenu->addSeparator();
  zoomMenu->addAction(export2CSVAction);
  zoomMenu->addAction(export2TXTAction);
  //鏃嬭浆鑿滃崟
  rotateMenu = menuBar()->addMenu(tr("测试"));
  rotateMenu->addAction(starCloudAction);
  rotateMenu->addAction(exportLogAction);
  rotateMenu->addAction(snapShotAction);
  //闀滃儚鑿滃崟
  mirrorMenu = menuBar()->addMenu(tr("测试"));
  mirrorMenu->addAction(outliersRemoveAction);
  mirrorMenu->addAction(filterAction);
}

void
PCLVisualizer::createToolBars()
{
  //点云文件工具栏
  fileTool = addToolBar("cloudFile");

  fileTool->addAction(newWorkStationAction);
  fileTool->addAction(addCloudAction);
  fileTool->addAction(newCloudAction);
  fileTool->addAction(copyCloudAction);
  fileTool->addAction(cutCloudAction);
  fileTool->addAction(pasteCloudAction);

  fileTool->addSeparator();

  fileTool->addAction(exportCloud2PCDAction);
  fileTool->addAction(exportCloud2PLYAction);
  fileTool->addAction(export2CSVAction);
  fileTool->addAction(export2TXTAction);
  fileTool->addAction(exportLogAction);
  fileTool->addAction(snapShotAction);

  fileTool->addSeparator();

  fileTool->addAction(starCloudAction);
  fileTool->addAction(searchCloudAction);
  fileTool->addAction(ui->actionBGColor);

  //算法工具栏
  algorithmTool = addToolBar("algorithm");
  algorithmTool->addAction(outliersRemoveAction);
  algorithmTool->addAction(filterAction);
  algorithmTool->addAction(alignAction);
  algorithmTool->addAction(MLSAction);
  algorithmTool->addAction(downSampleAction);
  algorithmTool->addAction(cloudSpliceAction);
  algorithmTool->addAction(HistogramAction);
  algorithmTool->addAction(surfaceAction);
}

void
PCLVisualizer::test()
{
  qDebug() << "Hello World!";
}

void
PCLVisualizer::initPointCloud()
{
  // Setup the cloud pointer
  cloud_.reset(new PointCloudT);
  // The number of points in the cloud
  cloud_->resize(800);

  // Fill the cloud with random points
  for (size_t i = 0; i < cloud_->points.size(); ++i) {
    cloud_->points[i].x = 1024 * rand() / (RAND_MAX + 1.0f);
    cloud_->points[i].y = 1024 * rand() / (RAND_MAX + 1.0f);
    cloud_->points[i].z = 1024 * rand() / (RAND_MAX + 1.0f);
  }

  pcl::getMinMax3D(*cloud_, p_min, p_max);
  maxLen = getMaxValue(p_max, p_min);
}

//连接信号槽
void
PCLVisualizer::connectSS()
{
  connect(ui->pushButton_inc,
          &QPushButton::clicked,
          this,
          &PCLVisualizer::IncPointSize);
  connect(ui->actionload_point_cloud,
          &QAction::triggered,
          this,
          &PCLVisualizer::loadPCDFile);
  connect(ui->actionsave_point_cloud,
          &QAction::triggered,
          this,
          &PCLVisualizer::savePCDFile);
  connect(ui->actionCoordinateSystem,
          &QAction::triggered,
          this,
          &PCLVisualizer::AddCoordinateSystem);
  // Connect "Load" and "Save" buttons and their functions
  connect(ui->pushButton_dec,
          &QPushButton::clicked,
          this,
          &PCLVisualizer::DecPointSize);

  // Connect X,Y,Z radio buttons and their functions
  connect(ui->radioButton_x,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseAxis);
  connect(ui->radioButton_y,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseAxis);
  connect(ui->radioButton_z,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseAxis);

  connect(ui->radioButton_BlueRed,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseColorMode);
  connect(ui->radioButton_GreenMagenta,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseColorMode);
  connect(ui->radioButton_WhiteRed,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseColorMode);
  connect(ui->radioButton_GreyRed,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseColorMode);
  connect(ui->radioButton_Rainbow,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseColorMode);
  connect(ui->radioButton_others,
          &QRadioButton::clicked,
          this,
          &PCLVisualizer::chooseColorMode);
  //增加新工作台功能
  connect(newWorkStationAction,
          &QAction::triggered,
          this,
          &PCLVisualizer::newWorkStation);
}

void
PCLVisualizer::savePCDFile()
{
  QString filename =
    QFileDialog::getSaveFileName(this,
                                 tr("Open point cloud"),
                                 "/home/",
                                 tr("Point cloud data(*.pcd *.ply)"));
  PCL_INFO("File chosen: %s\n", filename.toStdString().c_str());

  if (filename.isEmpty())
    return;
  int return_status;
  if (filename.endsWith(".pcd", Qt::CaseInsensitive))
    return_status = pcl::io::savePCDFileBinary(filename.toStdString(), *cloud_);
  else if (filename.endsWith(".ply", Qt::CaseInsensitive))
    return_status = pcl::io::savePLYFileBinary(filename.toStdString(), *cloud_);
  else {
    filename.append(".ply");
    return_status = pcl::io::savePLYFileBinary(filename.toStdString(), *cloud_);
  }
  if (return_status != 0) {
    PCL_ERROR("Error writing point cloud %s\n", filename.toStdString().c_str());
    return;
  }
}

void
PCLVisualizer::loadPCDFile()
{
  QString fileFormat, fileName, fileBaseName, pointCount, filePath, fileSuffix;
  QString filePathWithName =
    QFileDialog::getOpenFileName(this,
                                 tr("Open point cloud"),
                                 "/home/",
                                 tr("Point cloud data (*.pcd *.ply)"));
  QFileInfo fileInfo;
  fileInfo = QFileInfo(filePathWithName);
  //文件名
  fileName = fileInfo.fileName();
  //文件后缀
  fileSuffix = fileInfo.suffix();
  //绝对路径
  filePath = fileInfo.absolutePath();
  fileBaseName = fileInfo.baseName();
  qDebug() << fileName << endl
           << fileSuffix << endl
           << filePath << endl
           << fileInfo.baseName() << endl
           << fileInfo.completeBaseName();

  logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
           "] " + "[fiel choosing] " + filePathWithName.toStdString().c_str();
  ui->logList->addItem(logStr);
  PCL_WARN("File chosen: %s\n", filePathWithName.toStdString().c_str());
  //
  PointCloudT::Ptr cloud_tmp(new PointCloudT);
  //
  if (filePathWithName.isEmpty()) {
    logStr =
      "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
      "] " + "[file load failed] " + filePathWithName.toStdString().c_str();
    ui->logList->addItem(logStr);
    return;
  }

  logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
           "] " + "[file load success] " +
           filePathWithName.toStdString().c_str();
  ui->logList->addItem(logStr);

  int return_status;
  if (filePathWithName.endsWith(".pcd", Qt::CaseInsensitive)) {
    return_status =
      pcl::io::loadPCDFile(filePathWithName.toStdString(), *cloud_tmp);
    fileFormat = "PCD";
  } else {
    return_status =
      pcl::io::loadPLYFile(filePathWithName.toStdString(), *cloud_tmp);
    fileFormat = "PLY";
  }

  if (return_status != 0) {
    PCL_ERROR("Error reading point cloud %s\n",
              filePathWithName.toStdString().c_str());
    return;
  }
  PCL_WARN("file has loaded\n");

  // If point cloud contains NaN values, remove them before updating the
  // visualizer point cloud
  //    True if no points are invalid (e.g., have NaN or Inf values in any of
  //    their floating point fields).

  if (cloud_tmp->is_dense) {
    pcl::copyPointCloud(*cloud_tmp, *cloud_);
  } else {
    PCL_WARN("Cloud is not dense! Non finite points will be removed\n");
    std::vector<int> vec;
    pcl::removeNaNFromPointCloud(*cloud_tmp, *cloud_, vec);
  }
  qDebug() << "The number of points :" << cloud_->points.size();

  //更新点云属性信息
  ui->fileFormatEdt->setText(fileFormat);
  ui->fileNameEdt->setText(fileInfo.baseName());
  ui->pointCountEdt->setText(QString("%1").arg(cloud_->points.size()));
  QString cloudFile = fileName + "[" + filePath + "]";
  QListWidgetItem* item = new QListWidgetItem;
  item->setData(Qt::DisplayRole, cloudFile);
  item->setData(Qt::CheckStateRole, Qt::Checked);
  ui->filesList->addItem(item);

  //
  for (PointCloudT::iterator cloud_it = cloud_->begin();
       cloud_it != cloud_->end();
       ++cloud_it) {
    //    qDebug() << cloud_it->_PointXYZRGBA::r << " " <<
    //    cloud_it->_PointXYZRGBA::g
    //             << " " << cloud_it->_PointXYZRGBA::b << " "
    //             << cloud_it->_PointXYZRGBA::a;
    cloud_it->_PointXYZRGBA::a = 255;
  }

  pcl::getMinMax3D(*cloud_, p_min, p_max);
  maxLen = getMaxValue(p_max, p_min);

  colorCloudDistances();
  viewer_->updatePointCloud(cloud_, "cloud");
  viewer_->resetCamera();
  ui->qvtkWidget->update();
}

void
PCLVisualizer::chooseAxis()
{
  if (color_mode_ == 5)
    return;
  // Only 1 of the button can be checked at the time (mutual exclusivity) in a
  // group of radio buttons
  if (ui->radioButton_x->isChecked()) {
    PCL_INFO("x filtering chosen\n");
    filtering_axis_ = 0;
  } else if (ui->radioButton_y->isChecked()) {
    PCL_INFO("y filtering chosen\n");
    filtering_axis_ = 1;
  } else {
    PCL_INFO("z filtering chosen\n");
    filtering_axis_ = 2;
  }

  colorCloudDistances();
  viewer_->updatePointCloud(cloud_, "cloud");
  ui->qvtkWidget->update();
}

void
PCLVisualizer::chooseColorMode()
{
  // Only 1 of the button can be checked at the time (mutual exclusivity) in a
  // group of radio buttons
  if (ui->radioButton_BlueRed->isChecked()) {
    PCL_INFO("Blue -> Red LUT chosen\n");
    color_mode_ = 0;
  } else if (ui->radioButton_GreenMagenta->isChecked()) {
    PCL_INFO("Green -> Magenta LUT chosen\n");
    color_mode_ = 1;
  } else if (ui->radioButton_WhiteRed->isChecked()) {
    PCL_INFO("White -> Red LUT chosen\n");
    color_mode_ = 2;
  } else if (ui->radioButton_GreyRed->isChecked()) {
    PCL_INFO("Grey / Red LUT chosen\n");
    color_mode_ = 3;
  } else if (ui->radioButton_Rainbow->isChecked()) {
    PCL_INFO("Rainbow LUT chosen\n");
    color_mode_ = 4;
  } else {
    PCL_INFO("Full color chosen\n");
    color_mode_ = 5;
    QColor c = QColorDialog::getColor(Qt::red);
    if (c.isValid()) {
      point_color = c;
      qDebug() << "RBG: " << c.red() << " " << c.green() << " " << c.blue();
    }
  }

  colorCloudDistances();
  viewer_->updatePointCloud(cloud_, "cloud");
  ui->qvtkWidget->update();
}

void
PCLVisualizer::IncPointSize()
{
  viewer_->setPointCloudRenderingProperties(
    pcl::visualization::PCL_VISUALIZER_POINT_SIZE, ++point_size);

  qDebug() << "point_size = " << point_size;
  ui->label_pointSize->setText(QString::number(point_size));
  // viewer_->updatePointCloud(cloud_, "cloud");
  ui->qvtkWidget->update();
}

void
PCLVisualizer::DecPointSize()
{
  if (point_size == 1)
    return;
  viewer_->setPointCloudRenderingProperties(
    pcl::visualization::PCL_VISUALIZER_POINT_SIZE, --point_size);
  ui->label_pointSize->setText(QString::number(point_size));
  qDebug() << "point_size = " << point_size;
  // viewer_->updatePointCloud(cloud_, "cloud");
  ui->qvtkWidget->update();
}

void
PCLVisualizer::AddCoordinateSystem()
{
  QString str = ui->actionCoordinateSystem->text();
  if (str.compare("CoordinateSystem [OFF]") == 0) {
    qDebug() << str;
    ui->actionCoordinateSystem->setText("CoordinateSystem [ON]");
    viewer_->addCoordinateSystem();
  } else {
    ui->actionCoordinateSystem->setText("CoordinateSystem [OFF]");
    viewer_->removeCoordinateSystem();
  }
  ui->qvtkWidget->update();
}

//软件关闭时动作
void
PCLVisualizer::closeEvent(QCloseEvent* event)
{
  //写ini文件，记录当前窗口位置和大小：
  QString wstrFilePath =
    qApp->applicationDirPath() +
    "/setting.ini"; // in
                    // windows，我的工程名为EditPic，二editpic.ini放在工程源文件目录下
  QSettings* settings = new QSettings(
    wstrFilePath, QSettings::IniFormat); //用QSetting获取ini文件中的数据
  settings->clear();                     //清空当前配置文件中的内容
  settings->setValue("WindowGeometry/x", this->x());
  settings->setValue("WindowGeometry/y", this->y());
  settings->setValue("WindowGeometry/width", this->width());
  settings->setValue("WindowGeometry/height", this->height());
}

void
PCLVisualizer::colorCloudDistances()
{

  double min, max;
  switch (filtering_axis_) {
    case 0: // x
      min = cloud_->points[0].x;
      max = cloud_->points[0].x;
      break;
    case 1: // y
      min = cloud_->points[0].y;
      max = cloud_->points[0].y;
      break;
    default: // z
      min = cloud_->points[0].z;
      max = cloud_->points[0].z;
      break;
  }
  // Search for the minimum/maximum
  for (PointCloudT::iterator cloud_it = cloud_->begin();
       cloud_it != cloud_->end();
       ++cloud_it) {
    switch (filtering_axis_) {
      case 0: // x
        if (min > cloud_it->x)
          min = cloud_it->x;

        if (max < cloud_it->x)
          max = cloud_it->x;
        break;
      case 1: // y
        if (min > cloud_it->y)
          min = cloud_it->y;

        if (max < cloud_it->y)
          max = cloud_it->y;
        break;
      default: // z
        if (min > cloud_it->z)
          min = cloud_it->z;

        if (max < cloud_it->z)
          max = cloud_it->z;
        break;
    }
  }
  // Compute LUT scaling to fit the full histogram spectrum
  double lut_scale = 255.0 / (max - min); // max is 255, min is 0

  if (min ==
      max) // In case the cloud is flat on the chosen direction (x,y or z)
    lut_scale = 1.0; // Avoid rounding error in boost

  for (PointCloudT::iterator cloud_it = cloud_->begin();
       cloud_it != cloud_->end();
       ++cloud_it) {
    int value;
    switch (filtering_axis_) {
      case 0: // x
        value = boost::math::iround(
          (cloud_it->x - min) *
          lut_scale); // Round the number to the closest integer
        break;
      case 1: // y
        value = boost::math::iround((cloud_it->y - min) * lut_scale);
        break;
      default: // z
        value = boost::math::iround((cloud_it->z - min) * lut_scale);
        break;
    }

    // Apply color to the cloud
    switch (color_mode_) {
      case 0:
        // Blue (= min) -> Red (= max)
        cloud_it->r = value;
        cloud_it->g = 0;
        cloud_it->b = 255 - value;
        break;
      case 1:
        // Green (= min) -> Magenta (= max)
        cloud_it->r = value;
        cloud_it->g = 255 - value;
        cloud_it->b = value;
        break;
      case 2:
        // White (= min) -> Red (= max)
        cloud_it->r = 255;
        cloud_it->g = 255 - value;
        cloud_it->b = 255 - value;
        break;
      case 3:
        // Grey (< 128) / Red (> 128)
        if (value > 128) {
          cloud_it->r = 255;
          cloud_it->g = 0;
          cloud_it->b = 0;
        } else {
          cloud_it->r = 128;
          cloud_it->g = 128;
          cloud_it->b = 128;
        }
        break;
      case 5:
        cloud_it->r = point_color.red();
        cloud_it->g = point_color.green();
        cloud_it->b = point_color.blue();
        break;
      default:
        // Blue -> Green -> Red (~ rainbow)
        cloud_it->r =
          value > 128 ? (value - 128) * 2 : 0; // r[128] = 0, r[255] = 255
        cloud_it->g =
          value < 128
            ? 2 * value
            : 255 - ((value - 128) * 2); // g[0] = 0, g[128] = 255, g[255] = 0
        cloud_it->b =
          value < 128 ? 255 - (2 * value) : 0; // b[0] = 255, b[128] = 0
    }
  }
}

void
PCLVisualizer::on_actionUp_triggered()
{
  if (!cloud_->empty()) {
    viewer_->setCameraPosition(0.5 * (p_min.x + p_max.x),
                               0.5 * (p_min.y + p_max.y),
                               p_max.z + 2 * maxLen,
                               0.5 * (p_min.x + p_max.x),
                               0.5 * (p_min.y + p_max.y),
                               p_max.z,
                               0,
                               1,
                               0);
    ui->qvtkWidget->update();
  }
}

void
PCLVisualizer::on_actionBottom_triggered()
{
  if (!cloud_->empty()) {
    viewer_->setCameraPosition(0.5 * (p_min.x + p_max.x),
                               0.5 * (p_min.y + p_max.y),
                               p_min.z - 2 * maxLen,
                               0.5 * (p_min.x + p_max.x),
                               0.5 * (p_min.y + p_max.y),
                               p_min.z,
                               0,
                               1,
                               0);
    ui->qvtkWidget->update();
  }
}

void
PCLVisualizer::on_actionFront_triggered()
{
  if (!cloud_->empty()) {
    viewer_->setCameraPosition(0.5 * (p_min.x + p_max.x),
                               p_min.y - 2 * maxLen,
                               0.5 * (p_min.z + p_max.z),
                               0.5 * (p_min.x + p_max.x),
                               p_min.y,
                               0.5 * (p_min.z + p_max.z),
                               0,
                               0,
                               1);
    ui->qvtkWidget->update();
  }
}

void
PCLVisualizer::on_actionBack_triggered()
{
  if (!cloud_->empty()) {
    viewer_->setCameraPosition(0.5 * (p_min.x + p_max.x),
                               p_max.y + 2 * maxLen,
                               0.5 * (p_min.z + p_max.z),
                               0.5 * (p_min.x + p_max.x),
                               p_min.y,
                               0.5 * (p_min.z + p_max.z),
                               0,
                               0,
                               1);
    ui->qvtkWidget->update();
  }
}

void
PCLVisualizer::on_actionLeft_triggered()
{
  if (!cloud_->empty()) {
    viewer_->setCameraPosition(p_min.x - 2 * maxLen,
                               0.5 * (p_min.y + p_max.y),
                               0.5 * (p_min.z + p_max.z),
                               p_max.x,
                               0.5 * (p_min.y + p_max.y),
                               0.5 * (p_min.z + p_max.z),
                               0,
                               0,
                               1);
    ui->qvtkWidget->update();
  }
}

void
PCLVisualizer::on_actionRight_triggered()
{
  if (!cloud_->empty()) {
    viewer_->setCameraPosition(p_max.x + 2 * maxLen,
                               0.5 * (p_min.y + p_max.y),
                               0.5 * (p_min.z + p_max.z),
                               p_max.x,
                               0.5 * (p_min.y + p_max.y),
                               0.5 * (p_min.z + p_max.z),
                               0,
                               0,
                               1);
    ui->qvtkWidget->update();
  }
}

double
PCLVisualizer::getMinValue(PointT p1, PointT p2)
{
  double min = 0;

  if (p1.x - p2.x > p1.y - p2.y) {
    min = p1.y - p2.y;
  } else {
    min = p1.x - p2.x;
  }

  if (min > p1.z - p2.z) {
    min = p1.z - p2.z;
  }

  return min;
}

double
PCLVisualizer::getMaxValue(PointT p1, PointT p2)
{
  double max = 0;

  if (p1.x - p2.x > p1.y - p2.y) {
    max = p1.x - p2.x;

  } else {
    max = p1.y - p2.y;
  }

  if (max < p1.z - p2.z) {
    max = p1.z - p2.z;
  }

  return max;
}

void
PCLVisualizer::newWorkStation()
{
  PCLVisualizer* newPCV = new PCLVisualizer;
  //新建的工作窗口位于之前窗口的右下方
  newPCV->setGeometry(
    this->x() + 20, this->y() + 50, this->width(), this->height());
  newPCV->show();
}

void
PCLVisualizer::on_actionBGColor_triggered()
{
  QColor color = QColorDialog::getColor(
    bgColor, this); //打开颜色选择窗口，并用当前颜色初始化
  if (color.isValid()) {
    bgColor = color;
    qDebug() << "color: " << bgColor.red() << " " << bgColor.green() << " "
             << bgColor.blue();
    viewer_->setBackgroundColor(double(bgColor.red()) / 255,
                                double(bgColor.green()) / 255,
                                double(bgColor.blue()) / 255);
    ui->qvtkWidget->update();
  }
}
