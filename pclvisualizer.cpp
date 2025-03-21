﻿#pragma execution_character_set("utf-8")

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
#include "vtkGenericOpenGLRenderWindow.h"
#include <QThread>


PCLVisualizer::PCLVisualizer(QWidget* parent)
    : QMainWindow(parent)
      , isCloud2(true) // 设置初始点大小
      , point_size(5) // 设置初始背景颜色
      , ui(new Ui::PCLVisualizer) //是否默认开启RGBA显示点云
      , isRBGA(true) // 是否显示第二个点云
      , bgColor(0, 0, 50) // 初始化顺序要和声明的顺序一致
{
    ui->setupUi(this);
    //  cout << "INIT" << endl;
    initPCV();

    //  //创建动作，工具栏以及菜单栏
    createActions();
    //    createMenus();
    // 创建工具栏
    createToolBars();

    //初始化点云数据
    initPointCloud();

    // 给QVTK配置PCLViewer显示

    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    _renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow >::New();
    _renderWindow->AddRenderer(renderer);

    viewer_.reset(new pcl::visualization::PCLVisualizer(renderer, _renderWindow, "viewer", false));
    //设置背景颜色
    viewer_->setBackgroundColor(double(bgColor.red()) / 255,
                                double(bgColor.green()) / 255,
                                double(bgColor.blue()) / 255);

    ui->qvtkWidget->setRenderWindow(viewer_->getRenderWindow());
    viewer_->setupInteractor(ui->qvtkWidget->interactor(),
                             ui->qvtkWidget->renderWindow());
    ui->qvtkWidget->update();
    connectSS();

    // Color the randomly generated cloud
    // 以随机颜色填充点云
    colorCloudDistances();

    if (isRBGA)
    {
        viewer_->addPointCloud(cloudRGBA_, "cloud");
    }
    else
    {
        viewer_->addPointCloud(cloud_, "cloud");
    }
    ////viewer_->addPointCloud(cloud_, "cloud");
    // viewer_->addPointCloud(cloudRGBA_, "cloud");

    viewer_->setPointCloudRenderingProperties(
        pcl::visualization::PCL_VISUALIZER_POINT_SIZE, point_size);
    // viewer_->addCoordinateSystem(1);
    viewer_->resetCamera();
    viewer_->setLookUpTableID("cloud");
    ui->qvtkWidget->update();

    showLogItem("PCV 系统", "系统初始化完成。");
}

PCLVisualizer::~PCLVisualizer()
{
    delete ui;
}

void
PCLVisualizer::initPCV()
{
    //设置窗口名称
    QString str = "PointCloudViewer";
    this->setWindowTitle(str);

    showLogItem("PCV 系统", "系统正在初始化...");

    logStr = "主窗口位置已还原: " + QString("X-Y-Width-Height(%1,%2,%3,%4)")
                            .arg(this->x())
                            .arg(this->y())
                            .arg(this->width())
                            .arg(this->height());
    showLogItem("PCV 窗口", logStr);

    //写ini文件，记录当前窗口位置和大小：
    QString wstrFilePath =
        qApp->applicationDirPath() + "/setting.ini"; // .ini放在工程源文件目录下
    settings = new QSettings(
        wstrFilePath, QSettings::IniFormat); //用QSetting获取ini文件中的数据
    // settings->clear();                     //清空当前配置文件中的内容
}

void
PCLVisualizer::showLogItem(QString item, QString info)
{
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + " [ " + item + " ] " + info;
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::saveSetting(QString key, QString value)
{
    //写ini文件，记录当前窗口位置和大小：
    QString wstrFilePath =
        qApp->applicationDirPath() + "/setting.ini"; // .ini放在工程源文件目录下
    QSettings* settings = new QSettings(
        wstrFilePath, QSettings::IniFormat); //用QSetting获取ini文件中的数据
    // settings->clear();                     //清空当前配置文件中的内容
    settings->setValue(key, value);
    settings->sync();
}

QVariant
PCLVisualizer::getSetting(QString name)
{
    return settings->value(name);
}

void
PCLVisualizer::createActions()
{
    //添加点云
    // addCloudAction =
    // new QAction(QIcon(":/images/files/cloud.png"), "添加点云", this);
    // addCloudAction->setShortcut(tr("Ctrl+O"));    //(b)
    // addCloudAction->setStatusTip(tr("添加点云")); //(c)

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
}

void
PCLVisualizer::createToolBars()
{
    //点云文件工具栏
    fileTool = addToolBar("cloudFile");

    fileTool->addAction(newWorkStationAction);
    // fileTool->addAction(addCloudAction);
    fileTool->addAction(ui->actionload_point_cloud);
    fileTool->addAction(newCloudAction);
    fileTool->addAction(copyCloudAction);
    fileTool->addAction(cutCloudAction);
    fileTool->addAction(pasteCloudAction);

    fileTool->addSeparator();

    fileTool->addAction(exportCloud2PCDAction);
    fileTool->addAction(exportCloud2PLYAction);
    fileTool->addAction(export2CSVAction);
    fileTool->addAction(export2TXTAction);
    fileTool->addAction(ui->actionExportLog);
    fileTool->addAction(snapShotAction);

    fileTool->addSeparator();

    fileTool->addAction(starCloudAction);
    fileTool->addAction(searchCloudAction);
    fileTool->addAction(ui->actionBGColor);

    //算法工具栏
    algorithmTool = addToolBar("algorithm");
    algorithmTool->addAction(ui->actionbestRemoval);
    algorithmTool->addAction(ui->actionbestFiltering);
    algorithmTool->addAction(ui->actionbestKeypoint);
    algorithmTool->addAction(ui->actionbestRegistration);
    algorithmTool->addAction(ui->actionbestSurface);
    algorithmTool->addAction(MLSAction);
    algorithmTool->addAction(HistogramAction);
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
    cloud.reset(new PointCloudT);
    cloud_noise.reset(new PointCloudT);
    cloud_filtered.reset(new PointCloudT);
    cloud_filtered_guass.reset(new PointCloudT);
    cloud_filtered_guass_down.reset(new PointCloudT);
    cloud_filtered_out.reset(new PointCloudT);

    cloud_in.reset(new PointCloudT);
    cloud_tr.reset(new PointCloudT);
    cloud_RE.reset(new PointCloudT);

    cloudRGBA_.reset(new PointCloudTRGBA);

    // The number of points in the cloud
    cloud_->resize(1000);
    cloudRGBA_->resize(1000);
    // Fill the cloud with random points
    for (size_t i = 0; i < cloud_->points.size(); ++i)
    {
        cloud_->points[i].x = 1024 * rand() / (RAND_MAX + 1.0f);
        cloud_->points[i].y = 1024 * rand() / (RAND_MAX + 1.0f);
        cloud_->points[i].z = 1024 * rand() / (RAND_MAX + 1.0f);
    }

    // 获取点云内的最大点和最小点
    pcl::getMinMax3D(*cloud_, p_min, p_max);
    maxLen = getMaxValue(p_max, p_min);

    //拷贝一份给RGBA点云
    pcl::copyPointCloud(*cloud_, *cloudRGBA_);

    showLogItem("PCV 主窗口", "点云初始化完成");
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
    else
    {
        filename.append(".ply");
        return_status = pcl::io::savePLYFileBinary(filename.toStdString(), *cloud_);
    }
    if (return_status != 0)
    {
        PCL_ERROR("Error writing point cloud %s\n", filename.toStdString().c_str());
        return;
    }
}

void
PCLVisualizer::loadPCDFile()
{
    QString fileFormat, fileName, fileBaseName, pointCount, filePath, fileSuffix,
            lastPath;
    //读取文件名
    //记住上一次加载的路径

    lastPath = getSetting("FilePath/lastPath").toString();

    QString filePathWithName =
        QFileDialog::getOpenFileName(this,
                                     tr("Open point cloud"),
                                     lastPath,
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
    //qDebug() << fileName << endl
    //        << fileSuffix << endl
    //        << filePath << endl
    //        << fileInfo.baseName() << endl
    //        << fileInfo.completeBaseName();

    showLogItem("点云文件选择", filePath);
    // PCL_INFO("File chosen: %s\n", filePathWithName.toStdString().c_str());

    PointCloudT::Ptr cloud_tmp(new PointCloudT);

    if (filePathWithName.isEmpty())
    {
        showLogItem("文件加载失败", filePathWithName);
        return;
    }

    showLogItem("文件加载成功", filePathWithName);
    // 将当前文件路径保存，下次使用
    saveSetting("FilePath/lastPath", filePathWithName);

    //判断文件类型然后加载点云
    int return_status;
    if (filePathWithName.endsWith(".pcd", Qt::CaseInsensitive))
    {
        return_status =
            pcl::io::loadPCDFile(filePathWithName.toStdString(), *cloud_tmp);
        fileFormat = "PCD";
    }
    else
    {
        return_status =
            pcl::io::loadPLYFile(filePathWithName.toStdString(), *cloud_tmp);
        fileFormat = "PLY";
    }

    showLogItem("点云加载中", "文件格式为：" + fileFormat);

    //判断是否加载成功
    if (return_status != 0)
    {
        PCL_ERROR("Error reading point cloud %s\n",
                  filePathWithName.toStdString().c_str());

        showLogItem("点云加载中", fileName + " 文件读取失败。");
        return;
    }
    PCL_INFO("file has loaded\n");

    showLogItem("点云加载完成", fileFormat);

    // If point cloud contains NaN values, remove them before updating the
    // visualizer point cloud
    //    True if no points are invalid (e.g., have NaN or Inf values in any of
    //    their floating point fields).

    if (cloud_tmp->is_dense)
    {
        pcl::copyPointCloud(*cloud_tmp, *cloud_);
    }
    else
    {
        PCL_WARN("Cloud is not dense! Non finite points will be removed\n");
        std::vector<int> vec;
        pcl::removeNaNFromPointCloud(*cloud_tmp, *cloud_, vec);
    }
    //将当前点云拷贝给RGBA点云
    pcl::copyPointCloud(*cloud_, *cloudRGBA_);

    showLogItem("点云信息", fileName + QString(" 点云数量: %1").arg(cloud_->points.size()));

    qDebug() << "The number of points :" << cloud_->points.size();


    //更新点云属性信息
    ui->fileFormatEdt->setText(fileFormat);
    ui->fileNameEdt->setText(fileInfo.baseName());
    ui->pointCountEdt->setText(QString("%1").arg(cloud_->points.size()));
    QString cloudFile = fileName + " [" + filePath + "]";
    QListWidgetItem* item = new QListWidgetItem;
    //  item->setBackgroundColor(QColor(220, 230, 250));
    item->setBackground(QBrush(QColor(220, 230, 250)));
    item->setData(Qt::DisplayRole, cloudFile);
    item->setData(Qt::CheckStateRole, Qt::Checked);
    ui->filesList->addItem(item);

    //初始化 A
    for (PointCloudTRGBA::iterator cloud_it = cloudRGBA_->begin();
         cloud_it != cloudRGBA_->end();
         ++cloud_it)
    {
        //    qDebug() << cloud_it->_PointXYZRGBA::r << " " <<
        //    cloud_it->_PointXYZRGBA::g
        //             << " " << cloud_it->_PointXYZRGBA::b << " "
        //             << cloud_it->_PointXYZRGBA::a;
        cloud_it->_PointXYZRGBA::a = 255;
    }

    pcl::getMinMax3D(*cloud_, p_min, p_max);
    maxLen = getMaxValue(p_max, p_min);

    colorCloudDistances();

    if (isRBGA)
    {
        viewer_->updatePointCloud(cloudRGBA_, "cloud");
    }
    else
    {
        viewer_->updatePointCloud(cloud_, "cloud");
    }
    viewer_->resetCamera();
    viewer_->setPointCloudRenderingProperties(
        pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1);
    ui->qvtkWidget->update();
}

void
PCLVisualizer::loadPLYFile()
{
    QString fileFormat, fileName, fileBaseName, pointCount, filePath, fileSuffix;
    //读取文件名
    filePathWithName =
        QFileDialog::getOpenFileName(this,
                                     tr("Open point cloud"),
                                     "E:/BaiduNetdiskWorkspace/Paper-of-Luo/PCD",
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

    QString cloudFile = fileName + " [" + filePath + "]";
    QListWidgetItem* item = new QListWidgetItem;
    //  item->setBackgroundColor(QColor(220, 230, 250));
    item->setBackground(QBrush(QColor(220, 230, 250)));
    item->setData(Qt::DisplayRole, cloudFile);
    item->setData(Qt::CheckStateRole, Qt::Checked);
    ui->filesList->addItem(item);
}

void
PCLVisualizer::chooseAxis()
{
    if (color_mode_ == 5)
        return;
    // Only 1 of the button can be checked at the time (mutual exclusivity) in a
    // group of radio buttons
    if (ui->radioButton_x->isChecked())
    {
        PCL_INFO("x filtering chosen\n");
        filtering_axis_ = 0;
    }
    else if (ui->radioButton_y->isChecked())
    {
        PCL_INFO("y filtering chosen\n");
        filtering_axis_ = 1;
    }
    else
    {
        PCL_INFO("z filtering chosen\n");
        filtering_axis_ = 2;
    }

    colorCloudDistances();
    if (isRBGA)
    {
        viewer_->updatePointCloud(cloudRGBA_, "cloud");
    }
    else
    {
        viewer_->updatePointCloud(cloud_, "cloud");
    }
    ui->qvtkWidget->update();
}

void
PCLVisualizer::chooseColorMode()
{
    // Only 1 of the button can be checked at the time (mutual exclusivity) in a
    // group of radio buttons
    if (ui->radioButton_BlueRed->isChecked())
    {
        PCL_INFO("Blue -> Red LUT chosen\n");
        color_mode_ = 0;
    }
    else if (ui->radioButton_GreenMagenta->isChecked())
    {
        PCL_INFO("Green -> Magenta LUT chosen\n");
        color_mode_ = 1;
    }
    else if (ui->radioButton_WhiteRed->isChecked())
    {
        PCL_INFO("White -> Red LUT chosen\n");
        color_mode_ = 2;
    }
    else if (ui->radioButton_GreyRed->isChecked())
    {
        PCL_INFO("Grey / Red LUT chosen\n");
        color_mode_ = 3;
    }
    else if (ui->radioButton_Rainbow->isChecked())
    {
        PCL_INFO("Rainbow LUT chosen\n");
        color_mode_ = 4;
    }
    else
    {
        PCL_INFO("Full color chosen\n");
        color_mode_ = 5;
        QColor c = QColorDialog::getColor(Qt::red);
        if (c.isValid())
        {
            point_color = c;
            qDebug() << "RBG: " << c.red() << " " << c.green() << " " << c.blue();
        }
    }

    colorCloudDistances();
    if (isRBGA)
    {
        viewer_->updatePointCloud(cloudRGBA_, "cloud");
    }
    else
    {
        viewer_->updatePointCloud(cloud_, "cloud");
    }
    ui->qvtkWidget->update();
}

void
PCLVisualizer::IncPointSize()
{
    // setValue函数会自动调用 SpinBox的触发动作
    ui->pointSizeEdt->setValue(++point_size);
}

void
PCLVisualizer::DecPointSize()
{
    if (point_size == 1)
        return;
    ui->pointSizeEdt->setValue(--point_size);
}

void
PCLVisualizer::AddCoordinateSystem()
{
    QString str = ui->actionCoordinateSystem->text();
    if (str.compare("CoordinateSystem [OFF]") == 0)
    {
        qDebug() << str;
        ui->actionCoordinateSystem->setText("CoordinateSystem [ON]");
        viewer_->addCoordinateSystem();
    }
    else
    {
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
        qApp->applicationDirPath() + "/setting.ini"; // .ini放在工程源文件目录下
    QSettings* settings = new QSettings(
        wstrFilePath, QSettings::IniFormat); //用QSetting获取ini文件中的数据
    // settings->clear();                     //清空当前配置文件中的内容
    settings->setValue("WindowGeometry/x", this->x());
    settings->setValue("WindowGeometry/y", this->y());
    settings->setValue("WindowGeometry/width", this->width());
    settings->setValue("WindowGeometry/height", this->height());
    qDebug() << "Position is right:" << this->x() << " " << this->y() << " "
        << this->width() << " " << this->height();
}

void
PCLVisualizer::colorCloudDistances()
{
    double min, max;
    switch (filtering_axis_)
    {
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
    for (PointCloudTRGBA::iterator cloud_it = cloudRGBA_->begin();
         cloud_it != cloudRGBA_->end();
         ++cloud_it)
    {
        switch (filtering_axis_)
        {
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

    for (PointCloudTRGBA::iterator cloud_it = cloudRGBA_->begin();
         cloud_it != cloudRGBA_->end();
         ++cloud_it)
    {
        int value;
        switch (filtering_axis_)
        {
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
        switch (color_mode_)
        {
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
            if (value > 128)
            {
                cloud_it->r = 255;
                cloud_it->g = 0;
                cloud_it->b = 0;
            }
            else
            {
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
    if (!cloud_->empty())
    {
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
    if (!cloud_->empty())
    {
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
    if (!cloud_->empty())
    {
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
    if (!cloud_->empty())
    {
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
    if (!cloud_->empty())
    {
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
    if (!cloud_->empty())
    {
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

    if (p1.x - p2.x > p1.y - p2.y)
    {
        min = p1.y - p2.y;
    }
    else
    {
        min = p1.x - p2.x;
    }

    if (min > p1.z - p2.z)
    {
        min = p1.z - p2.z;
    }
    return min;
}

double
PCLVisualizer::getMaxValue(PointT p1, PointT p2)
{
    double max = 0;

    if (p1.x - p2.x > p1.y - p2.y)
    {
        max = p1.x - p2.x;
    }
    else
    {
        max = p1.y - p2.y;
    }

    if (max < p1.z - p2.z)
    {
        max = p1.z - p2.z;
    }

    return max;
}

void
PCLVisualizer::openProgressDlg(int num = 500)
{
    //创建一个进度对话框
    QProgressDialog* progressDialog = new QProgressDialog(this);
    QFont font("ZYSong18030", 12);
    progressDialog->setFont(font);
    progressDialog->setWindowModality(Qt::WindowModal); //(d)
    progressDialog->setMinimumDuration(5); //(e)
    progressDialog->setWindowTitle(tr("Please Wait")); //(f)
    progressDialog->setLabelText(tr("Processing...")); //(g)
    progressDialog->setCancelButtonText(tr("Cancel")); //(h)
    progressDialog->setRange(0, num); //设置进度对话框的步进范围
    for (int i = 1; i < num + 1; i++)
    {
        QThread::sleep(10);
        progressDialog->setValue(i); //(i)
        if (progressDialog->wasCanceled()) //(j)
            return;
    }
}

void
PCLVisualizer::updateCloudInfo()
{
    // TODO 暂时只更新点云数量
    ui->pointCountEdt->setText(QString::number(cloud_->points.size()));
}

void
PCLVisualizer::best_filter()
{
    pcl::console::TicToc time;

    //创建一个进度对话框
    QProgressDialog* progressDialog = new QProgressDialog(this);
    QFont font("ZYSong18030", 12);
    progressDialog->setFont(font);
    progressDialog->setWindowModality(Qt::WindowModal); //(d)
    progressDialog->setMinimumDuration(10); //(e)
    progressDialog->setWindowTitle(tr("Please Wait")); //(f)
    progressDialog->setLabelText(tr("Processing...")); //(g)
    progressDialog->setCancelButtonText(tr("Cancel")); //(h)
    progressDialog->setRange(0, 100); //设置进度对话框的步进范围

    progressDialog->setValue(10); //(i)
    if (progressDialog->wasCanceled()) //(j)
        return;

    *cloud = *cloud_;
    //添加高斯噪声
    time.tic();
    GenerateGaussNoise(cloud, cloud_noise, 0, 0.001);
    std::cout << "添加高斯噪声耗时: " << time.toc() << " ms" << std::endl;
    std::cerr << "添加高斯噪声: " << std::endl;
    std::cerr << *cloud_noise << std::endl;
    // show_point_cloud(cloud_noise, "point cloud with noise");

    progressDialog->setValue(20); //(i)
    if (progressDialog->wasCanceled()) //(j)
        return;
    //离群点移除
    time.tic();
    radius_filter(cloud_noise, cloud_filtered, cloud_filtered_out);
    // statistical_filter(cloud_noise, cloud_filtered, cloud_filtered_out);
    // std::cout << "离群点移除耗时: " << time.toc() << " ms" << std::endl;
    // std::cerr << "离群点移除: " << std::endl;
    // std::cerr << *cloud_filtered << std::endl;

    //--------------------LOG--------------------------
    logStr =
        "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "] " +
        QString("[离群点移除] Outliers Remove use time : %1").arg(time.toc()) +
        " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    progressDialog->setValue(40); //(i)
    if (progressDialog->wasCanceled()) //(j)
        return;

    //高斯平滑
    time.tic();
    gaussian_filter(cloud_filtered, cloud_filtered_guass, 0.01);
    // std::cout << "高斯平滑耗时: " << time.toc() << " ms" << std::endl;
    // std::cerr << "高斯平滑: " << std::endl;
    // std::cerr << *cloud_filtered_guass << std::endl;

    //--------------------LOG--------------------------
    logStr =
        "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "] " +
        QString("[滤波平滑] Filtering use time : %1").arg(time.toc()) + " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    progressDialog->setValue(60); //(i)
    if (progressDialog->wasCanceled()) //(j)
        return;

    //下采样
    time.tic();
    downSampling(cloud_filtered_guass, cloud_filtered_guass_down, 0.01);
    // std::cout << "下采样耗时: " << time.toc() << " ms" << std::endl;
    // std::cerr << "下采样: " << std::endl;
    // std::cerr << *cloud_filtered_guass_down << std::endl;

    //--------------------LOG--------------------------
    logStr =
        "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "] " +
        QString("[下采样] Down Sampling use time : %1").arg(time.toc()) + " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    progressDialog->setValue(80); //(i)
    if (progressDialog->wasCanceled()) //(j)
        return;

    writer.write<pcl::PointXYZ>("cloud_noise.pcd", *cloud_noise, false);
    writer.write<pcl::PointXYZ>("cloud_filtered.pcd", *cloud_filtered, false);
    writer.write<pcl::PointXYZ>(
        "cloud_filtered_guass.pcd", *cloud_filtered_guass, false);
    writer.write<pcl::PointXYZ>(
        "cloud_filtered_guass_down.pcd", *cloud_filtered_guass_down, false);

    // text_vector.push_back("cloud");
    // text_vector.push_back("cloud_noise");
    // text_vector.push_back("cloud_filtered");
    // text_vector.push_back("cloud_filtered_guass");
    // text_vector.push_back("cloud_filtered_guass_down");
    // cloudPtr_vector.push_back(cloud);
    // cloudPtr_vector.push_back(cloud_noise);
    // cloudPtr_vector.push_back(cloud_filtered);
    // cloudPtr_vector.push_back(cloud_filtered_guass);
    // cloudPtr_vector.push_back(cloud_filtered_guass_down);

    // TODO RGB点云暂不更新

    *cloud_ = *cloud_filtered_guass_down;
    // if (isRBGA) {
    //	viewer_->updatePointCloud(cloudRGBA_, "cloud");
    //}
    // else {
    //	viewer_->updatePointCloud(cloud_, "cloud");
    //}
    viewer_->updatePointCloud(cloud_, "cloud");
    viewer_->resetCamera();
    ui->qvtkWidget->update();
    //更新点云信息
    updateCloudInfo();

    progressDialog->setValue(100); //(i)
    if (progressDialog->wasCanceled()) //(j)
        return;

    return;
}

void
PCLVisualizer::cloudTransform(PointCloudT::Ptr cloud_in,
                              PointCloudT::Ptr cloud_tr,
                              double theta,
                              double z)
{
    //设置旋转矩阵和平移向量
    Eigen::Matrix4d transformation_matrix = Eigen::Matrix4d::Identity();

    //旋转角度
    // double theta = M_PI / 4;  // The angle of rotation in radians
    transformation_matrix(0, 0) = cos(theta);
    transformation_matrix(0, 1) = -sin(theta);
    transformation_matrix(1, 0) = sin(theta);
    transformation_matrix(1, 1) = cos(theta);

    double x = ui->lineEdit_X->text().toDouble();
    double y = ui->lineEdit_Y->text().toDouble();
    double zz = ui->lineEdit_Z->text().toDouble();
    qDebug() << "x:" << x << " y:" << y << " z:" << zz;
    transformation_matrix(0, 3) = x; // 0.1m
    transformation_matrix(1, 3) = y; // 0.1m
    transformation_matrix(2, 3) = zz; // 0.1m
    // transformation_matrix(2, 3) = z; // 0.1m
    // Z轴方向上的平移
    // A translation on Z axis (0.4 meters)
    // transformation_matrix(2, 3) = z; // 0.1m

    // Display in terminal the transformation matrix
    // std::cout << "对源点云进行旋转平移：" << std::endl;
    print4x4Matrix(transformation_matrix);

    //将原始点云先旋转平移
    // Executing the transformation
    pcl::transformPointCloud(*cloud_in, *cloud_tr, transformation_matrix);
}

void
PCLVisualizer::ICP_aligin(pcl::IterativeClosestPoint<PointT, PointT>::Ptr icp,
                          PointCloudT::Ptr cloud_in,
                          PointCloudT::Ptr cloud_RE)
{
    icp->setMaximumIterations(1);
    icp->setInputSource(cloud_RE);
    icp->setInputTarget(cloud_in);
    icp->align(*cloud_RE);
    icp->setMaximumIterations(1); // We set this variable to 1 for the next time
    // we will call .align () function
    std::cout << "Applied " << iterations << " iteration(s)" << std::endl;
    if (icp->hasConverged())
    {
        std::cout << "\nHasConverged: " << icp->hasConverged()
            << ", getFitnessScore: " << icp->getFitnessScore() << std::endl;
        cout << "Transformation: \n" << icp->getFinalTransformation() << endl;
        print4x4Matrix(icp->getFinalTransformation().cast<double>());
        qDebug() << get4x4MatrixStr(icp->getFinalTransformation().cast<double>());
        ////ui->matEdit.setText(get4x4MatrixStr(icp->getFinalTransformation().cast<double>()));
        ui->matEdit->setPlainText(
            get4x4MatrixStr(icp->getFinalTransformation().cast<double>()));
    }
    else
    {
        PCL_ERROR("\nICP has not converged.\n");
        // return (-1);
    }
}

void
PCLVisualizer::best_aligin()
{
    boost::make_shared<pcl::IterativeClosestPoint<PointT, PointT>>();

    *cloud_in = *cloud_;
    //将原始点云旋转平移
    cloudTransform(cloud_in, cloud_RE, 0, 10);
    *cloud_tr = *cloud_RE; // 在cloud_tr中备份，以供显示

    pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_in_color_h(
        cloud_in, 20, 20, 180);
    pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_tr_color_h(
        cloud_tr, 250, 80, 0);
    pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_icp_color_h(
        cloud_RE, 180, 20, 20);
    if (isCloud2)
    {
        viewer_->addPointCloud(cloud_tr, cloud_tr_color_h, "cloud2");
        isCloud2 = false;
    }
    else
    {
        viewer_->updatePointCloud(cloud_tr, cloud_tr_color_h, "cloud2");
    }

    viewer_->updatePointCloud(cloud_in, cloud_in_color_h, "cloud");
    viewer_->resetCamera();
    ui->qvtkWidget->update();
}

void
PCLVisualizer::best_surface()
{
    *cloud_in = *cloud_;
    pcl::PolygonMesh mesh; //存储最终三角化的网格模型
    pcl::PolygonMesh mesh_mls; //存储最终三角化的网格模型

    //下采样精简点云
    Voxel_downsampling(cloud_in, cloud, 0);

    //-----------------连接XYZ和法向量字段--------------
    pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(
        new pcl::PointCloud<pcl::PointNormal>);
    pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals_mls(
        new pcl::PointCloud<pcl::PointNormal>);

    bool isMLS = false;

    if (isMLS)
    {
        MLS(cloud, cloud_with_normals_mls);
        GP(cloud, cloud_with_normals_mls, mesh);
    }
    else
    {
        //----------------法线估计-------------------------
        pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> n;
        pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
        pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(
            new pcl::search::KdTree<pcl::PointXYZ>);
        tree->setInputCloud(cloud);
        n.setInputCloud(cloud);
        n.setSearchMethod(tree);
        n.setKSearch(10);
        n.compute(*normals);

        pcl::concatenateFields(*cloud, *normals, *cloud_with_normals);
        GP(cloud, cloud_with_normals, mesh);
    }
    viewer_->removePointCloud("cloud");
    viewer_->removePointCloud("cloud2");
    viewer_->addPolygonMesh(mesh, "mesh");
    viewer_->resetCamera();
    ui->qvtkWidget->update();
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
    if (color.isValid())
    {
        bgColor = color;
        qDebug() << "color: " << bgColor.red() << " " << bgColor.green() << " "
            << bgColor.blue();
        viewer_->setBackgroundColor(double(bgColor.red()) / 255,
                                    double(bgColor.green()) / 255,
                                    double(bgColor.blue()) / 255);
        ui->qvtkWidget->update();
    }
}

void
PCLVisualizer::on_actionabout_triggered()
{
}

void
PCLVisualizer::on_comboBox_Color_currentIndexChanged(const QString& arg1)
{
    isRBGA = (arg1 == "RGB");
    if (isRBGA)
    {
        viewer_->updatePointCloud(cloudRGBA_, "cloud");
    }
    else
    {
        viewer_->updatePointCloud(cloud_, "cloud");
    }
    ui->qvtkWidget->update();
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + QString("[点云颜色模式] Change to %1").arg(arg1);
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::on_actionCoordinateSystem_triggered()
{
}

void
PCLVisualizer::on_actionCameraview_triggered()
{
}

void
PCLVisualizer::on_pointSizeEdt_valueChanged(int arg1)
{
    point_size = arg1;
    ui->pointSizeEdt->setValue(arg1);
    viewer_->setPointCloudRenderingProperties(
        pcl::visualization::PCL_VISUALIZER_POINT_SIZE, point_size);
    ui->label_pointSize->setText(QString::number(point_size));
    qDebug() << "point_size = " << point_size;
    ui->qvtkWidget->update();

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + QString("[数据点大小] POINT_SIZE: %1").arg(arg1);
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::on_actionbestSurface_triggered()
{
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[表面重建] surface rebuild start";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    q_time.start();

    openProgressDlg(300);
    best_surface();

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[表面重建] surface rebuild Done";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    //--------------------LOG--------------------------
    logStr =
        "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "] " +
        QString("[表面重建] surface rebuild use time : %1").arg(q_time.elapsed()) +
        " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::on_actionbestRemoval_triggered()
{
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[去噪平滑] Outliers Remove start";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    q_time.start();
    // openProgressDlg(300);
    best_filter();

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[去噪平滑] Outliers Remove Done";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    //--------------------LOG--------------------------
    logStr =
        "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "] " +
        QString("[去噪平滑] Outliers Remove use time : %1").arg(q_time.elapsed()) +
        " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::on_actionbestFiltering_triggered()
{
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[滤波平滑] surface filtering start";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    q_time.start();
    openProgressDlg(300);
    Voxel_downsampling(cloud_, cloud_, 0.001);

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[滤波平滑] surface filtering Done";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " +
        QString("[滤波平滑] surface filtering use time : %1")
        .arg(q_time.elapsed()) +
        " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    updateCloudInfo();
    viewer_->updatePointCloud(cloud_, "cloud");
    viewer_->resetCamera();
    ui->qvtkWidget->update();
}

void
PCLVisualizer::on_actionbestRegistration_triggered()
{
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[点云配准] Cloud Registration start";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
    q_time.start();
    // openProgressDlg(300);
    best_aligin();

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[点云配准] Cloud Registration Done";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " +
        QString("[点云配准] Cloud Registration use time : %1")
        .arg(q_time.elapsed()) +
        " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::on_actionbestKeypoint_triggered()
{
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[关键点提取] Key points extracting start";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    q_time.start();
    openProgressDlg(300);
    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[关键点提取] Key points extracting Done";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " +
        QString("[关键点提取] Key points extracting use time : %1")
        .arg(q_time.elapsed()) +
        " ms";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------
}

void
PCLVisualizer::on_actionTXT_triggered()
{
}

void
PCLVisualizer::on_actionExportLog_triggered()
{
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Save Log"), "/home/", tr("Log(*.txt)"));
    PCL_INFO("File chosen: %s\n", filename.toStdString().c_str());

    QFile file;
    file.setFileName(filename);
    QByteArray data;

    QString log;
    for (auto it = logList.begin(); it != logList.end(); ++it)
    {
        log.append(*it + "\n");
    }
    if (file.open(QIODevice::WriteOnly))
    {
        QByteArray res2 = log.toUtf8(); // toLatin1()转为QByteArray
        file.write(res2);
        file.close();
    }
}

void
PCLVisualizer::on_actionRedo_triggered()
{
    ++iterations;
    //  if (iterations < 3) {
    //    openProgressDlg(300);
    //  }

    // qDebug() << "iterations: "<< iterations << endl;
    // TODO 暂时在这里做点云配准显示

    ICP_aligin(icp, cloud_in, cloud_RE);

    pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_in_color_h(
        cloud_in, 20, 20, 180);
    pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_tr_color_h(
        cloud_tr, 250, 80, 0);
    pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_icp_color_h(
        cloud_RE, 180, 20, 20);

    viewer_->updatePointCloud(cloud_RE, cloud_icp_color_h, "cloud2");
    viewer_->updatePointCloud(cloud_in, cloud_in_color_h, "cloud");
    viewer_->resetCamera();
    ui->qvtkWidget->update();
}

void
PCLVisualizer::on_actionquit_triggered()
{
}

double
obb(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud) //点云OBB有向包围盒
{
    pcl::MomentOfInertiaEstimation<pcl::PointXYZ> feature_extractor;
    feature_extractor.setInputCloud(cloud);
    feature_extractor.compute();

    std::vector<float> moment_of_inertia;
    std::vector<float> eccentricity;
    pcl::PointXYZ min_point_OBB;
    pcl::PointXYZ max_point_OBB;
    pcl::PointXYZ position_OBB;
    Eigen::Matrix3f rotational_matrix_OBB;
    float major_value, middle_value, minor_value;
    Eigen::Vector3f major_vector, middle_vector, minor_vector;
    Eigen::Vector3f mass_center;

    feature_extractor.getMomentOfInertia(moment_of_inertia);
    feature_extractor.getEccentricity(eccentricity);
    feature_extractor.getOBB(
        min_point_OBB, max_point_OBB, position_OBB, rotational_matrix_OBB);
    feature_extractor.getEigenValues(major_value, middle_value, minor_value);
    feature_extractor.getEigenVectors(major_vector, middle_vector, minor_vector);
    feature_extractor.getMassCenter(mass_center);

    return (max_point_OBB.x - min_point_OBB.x) *
        (max_point_OBB.y - min_point_OBB.y) *
        (max_point_OBB.z - min_point_OBB.z);

    ////绘制OBB包围盒
    // pcl::visualization::PCLVisualizer::Ptr viewer(new
    // pcl::visualization::PCLVisualizer("3D Viewer"));
    // viewer->setBackgroundColor(0, 0, 0);
    ////viewer->addCoordinateSystem(1.0);

    // pcl::visualization::PointCloudColorHandlerRandom<pcl::PointXYZ>
    // RandomColor(cloud);//设置随机颜色
    // viewer->addPointCloud<pcl::PointXYZ>(cloud, RandomColor, "points");
    // viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE,
    // 3, "points");

    // Eigen::Vector3f position(position_OBB.x, position_OBB.y, position_OBB.z);
    // Eigen::Quaternionf quat(rotational_matrix_OBB);
    // viewer->addCube(position, quat, max_point_OBB.x - min_point_OBB.x,
    // max_point_OBB.y - min_point_OBB.y, max_point_OBB.z - min_point_OBB.z,
    // "OBB");
    // viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION,
    // pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "OBB");

    // pcl::PointXYZ center(mass_center(0), mass_center(1), mass_center(2));
    // pcl::PointXYZ x_axis(major_vector(0) + mass_center(0), major_vector(1) +
    // mass_center(1), major_vector(2) + mass_center(2)); pcl::PointXYZ
    // y_axis(middle_vector(0) + mass_center(0), middle_vector(1) +
    // mass_center(1), middle_vector(2) + mass_center(2)); pcl::PointXYZ
    // z_axis(minor_vector(0) + mass_center(0), minor_vector(1) + mass_center(1),
    // minor_vector(2) + mass_center(2)); viewer->addLine(center, x_axis, 1.0f,
    // 0.0f, 0.0f, "major eigen vector");//主成分 viewer->addLine(center, y_axis,
    // 0.0f, 1.0f, 0.0f, "middle eigen vector"); viewer->addLine(center, z_axis,
    // 0.0f, 0.0f, 1.0f, "minor eigen vector");

    // std::cout << mass_center << std::endl;//中心点
    // std::cout << rotational_matrix_OBB << std::endl;//矩阵

    // while (!viewer->wasStopped())
    //{
    //	viewer->spinOnce(100);
    //}
}

double
aabb(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud) //点云AABB包围盒
{
    pcl::MomentOfInertiaEstimation<pcl::PointXYZ> feature_extractor;
    feature_extractor.setInputCloud(cloud);
    feature_extractor.compute();

    std::vector<float> moment_of_inertia;
    std::vector<float> eccentricity;

    pcl::PointXYZ min_point_AABB; // AABB包围盒
    pcl::PointXYZ max_point_AABB;

    Eigen::Vector3f major_vector, middle_vector, minor_vector;
    Eigen::Vector3f mass_center;

    feature_extractor.getMomentOfInertia(moment_of_inertia);
    feature_extractor.getEccentricity(eccentricity);
    feature_extractor.getAABB(min_point_AABB, max_point_AABB);
    feature_extractor.getEigenVectors(major_vector, middle_vector, minor_vector);
    feature_extractor.getMassCenter(mass_center);

    return (max_point_AABB.x - min_point_AABB.x) *
        (max_point_AABB.y - min_point_AABB.y) *
        (max_point_AABB.z - min_point_AABB.z);
    ////绘制AABB包围盒
    // pcl::visualization::PCLVisualizer::Ptr viewer(new
    // pcl::visualization::PCLVisualizer("3D Viewer"));
    // viewer->setBackgroundColor(0, 0, 0);
    ////viewer->addCoordinateSystem(1.0);

    // pcl::visualization::PointCloudColorHandlerRandom<pcl::PointXYZ>
    // RandomColor(cloud);//设置随机颜色
    // viewer->addPointCloud<pcl::PointXYZ>(cloud, RandomColor, "points");
    // viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE,
    // 3, "points");

    // viewer->addCube(min_point_AABB.x, max_point_AABB.x, min_point_AABB.y,
    // max_point_AABB.y, min_point_AABB.z, max_point_AABB.z, 1.0, 1.0, 0.0,
    // "AABB");
    // viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION,
    // pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "AABB");

    // pcl::PointXYZ center(mass_center(0), mass_center(1), mass_center(2));
    // pcl::PointXYZ x_axis(major_vector(0) + mass_center(0), major_vector(1) +
    // mass_center(1), major_vector(2) + mass_center(2)); pcl::PointXYZ
    // y_axis(middle_vector(0) + mass_center(0), middle_vector(1) +
    // mass_center(1), middle_vector(2) + mass_center(2)); pcl::PointXYZ
    // z_axis(minor_vector(0) + mass_center(0), minor_vector(1) + mass_center(1),
    // minor_vector(2) + mass_center(2)); viewer->addLine(center, x_axis, 1.0f,
    // 0.0f, 0.0f, "major eigen vector"); viewer->addLine(center, y_axis,
    // 0.0f, 1.0f, 0.0f, "middle eigen vector"); viewer->addLine(center, z_axis,
    // 0.0f, 0.0f, 1.0f, "minor eigen vector");

    // while (!viewer->wasStopped())
    //{
    //	viewer->spinOnce(100);
    //}
}

void
PCLVisualizer::on_actiongetAllGeo_triggered()
{
    vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(filePathWithName.toUtf8());
    reader->Update();
    vtkSmartPointer<vtkTriangleFilter> tri =
        vtkSmartPointer<vtkTriangleFilter>::New();
    tri->SetInputData(reader->GetOutput());
    tri->Update();
    vtkSmartPointer<vtkMassProperties> poly =
        vtkSmartPointer<vtkMassProperties>::New();
    poly->SetInputData(tri->GetOutput());
    poly->Update();

    double pV = poly->GetVolumeProjected();
    double pX = poly->GetVolumeX();
    double pY = poly->GetVolumeY();
    double pZ = poly->GetVolumeZ();
    double vol = poly->GetVolume(); //体积
    double area = poly->GetSurfaceArea(); //表面积
    double maxArea = poly->GetMaxCellArea(); //最大单元面积
    double minArea = poly->GetMinCellArea(); //最小单元面积

    pcl::PLYReader PLY_reader;
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
    PLY_reader.read<pcl::PointXYZ>(filePathWithName.toStdString(), *cloud);

    double vol_aabb = aabb(cloud);
    double vol_obb = obb(cloud);

    ui->lineEdit_AABB->setText(QString::number(vol_aabb));
    ui->lineEdit_OBB->setText(QString::number(vol_obb));
    ui->lineEdit_area->setText(QString::number(area));
    ui->lineEdit_maxUnitAera->setText(QString::number(maxArea));
    ui->lineEdit_minUnitAera->setText(QString::number(minArea));
    ui->lineEdit_vol->setText(QString::number(vol));

    // QMessageBox::information(
    //  this, "几何属性提取成功", "表面积、体积等计算完成", "确定");

    cout << "vol: " << vol << endl;
    cout << "area: " << area << endl;
    cout << "maxArea: " << maxArea << endl;
    cout << "minArea: " << minArea << endl;
    cout << "pV: " << pV << endl;
    cout << "pX: " << pX << endl;
    cout << "pY: " << pY << endl;
    cout << "pZ: " << pZ << endl;
}

void
PCLVisualizer::on_actionplaneSeg_triggered()
{
    // ----------------------------加载点云-----------------------------
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PCDWriter writer;
    //  if (pcl::io::loadPCDFile<pcl::PointXYZ>(
    //        "E:\\PCLProject\\pcl-project\\pcl_segmentation\\plane\\cmake_"
    //        "bin\\table_scene_lms400_downSampling.pcd",
    //        *cloud) == -1) {
    //    PCL_ERROR("读取源标点云失败 \n");
    //  }
    *cloud = *cloud_;
    cout << "从点云中读取 " << cloud->size() << " 个点" << endl;

    //第一步：定义输入的原始数据以及分割获得的点、平面系数coefficients、存储内点的索引集合对象inliers
    pcl::PointCloud<pcl::PointXYZ>::Ptr planar_segment(
        new pcl::PointCloud<pcl::PointXYZ>); //创建分割对象
    pcl::ModelCoefficients::Ptr coefficients(
        new pcl::ModelCoefficients); //模型系数
    pcl::PointIndices::Ptr inliers(new pcl::PointIndices); //索引列表
    pcl::SACSegmentation<pcl::PointXYZ> seg; //分割对象

    pcl::ExtractIndices<pcl::PointXYZ> extract; //提取器

    int n_piece = 2; //需要探测的面的个数

    //第二步：加载原始点云到可视化窗口

    //    viewer->setBackgroundColor(0, 0, 0.2);
    pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> single_color(
        cloud, 255, 255, 255);
    viewer_->updatePointCloud<pcl::PointXYZ>(cloud, single_color, "cloud");
    // viewer_->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE,
    // 1, "sample"); 第三步：使用RANSAC获取点数最多的面
    for (int i = 0; i < n_piece; i++)
    {
        seg.setOptimizeCoefficients(true); //使用内部点重新估算模型参数
        seg.setModelType(pcl::SACMODEL_PLANE); //设置模型类型
        seg.setMethodType(pcl::SAC_RANSAC); //设置随机采样一致性方法类型
        seg.setDistanceThreshold(
            0.01); //设定距离阀值，距离阀值决定了点被认为是局内点是必须满足的条件
        seg.setInputCloud(cloud);
        seg.segment(*inliers, *coefficients);

        extract.setInputCloud(cloud);
        extract.setIndices(inliers);
        extract.setNegative(false);
        //提取探测出来的平面
        extract.filter(*planar_segment);
        // planar_segment为该次探测出来的面片，可以单独进行保存，此处省略

        //剔除探测出的平面，在剩余点中继续探测平面
        extract.setNegative(true);
        extract.filter(*cloud);

        int R;
        int G;
        int B;
        if (i == 0)
        {
            R = 0;
            G = 255;
            B = 0;
        }
        else
        {
            R = 255;
            G = 0;
            B = 0;
        }
        std::stringstream ss;
        ss << "planar_segment_" << (i + 1) << ".pcd";
        std::string str;
        ss >> str;
        writer.write<pcl::PointXYZ>(str, *planar_segment, false);

        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> plane_color(
            planar_segment, R, G, B);
        viewer_->addPointCloud<pcl::PointXYZ>(planar_segment, plane_color, str);
        viewer_->setPointCloudRenderingProperties(
            pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, str);

        viewer_->resetCamera();
        ui->qvtkWidget->update();
    }

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[PCV 主窗口] " + "Point Cloud Plane Segmentation Done.";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    //--------------------LOG--------------------------
    logStr = "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
        "] " + "[PCV 主窗口] " + "PCV has found 2 planes.";
    logList.push_back(logStr);
    ui->logList->addItem(logStr);
    //--------------------LOG--------------------------

    // QString info = "搜索到两个平面，是否进行平面分割？";
    QMessageBox::information(
        this, "平面分割成功", "搜索到2个平面，是否进行提取？", "确定", "取消");
}

//显示PLY文件
void
PCLVisualizer::on_actionarea_triggered()
{
    loadPLYFile();
    // pcl::PolygonMesh mesh;
    // pcl::io::loadPLYFile(filePathWithName.toStdString(), mesh);
    // viewer_->removePointCloud("cloud");
    // viewer_->addPolygonMesh(mesh, "my");
    // viewer_->resetCamera();
    // ui->qvtkWidget->update();
}

void
PCLVisualizer::on_actionvol_triggered()
{
    QMessageBox::information(
        this, "几何属性提取成功", "表面积、体积等计算完成", "确定");
}
