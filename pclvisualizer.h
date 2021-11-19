#ifndef PCLVISUALIZER_H
#define PCLVISUALIZER_H

#include <QAction>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFontComboBox>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QRect>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QTextCharFormat>
#include <QToolBar>
#include <QToolButton>

// Point Cloud Library
#include <pcl/common/common.h>
#include <pcl/filters/filter.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>

#include <QColorDialog>
// Boost
#include <boost/math/special_functions/round.hpp>

// Visualization Toolkit (VTK)
#include <vtkRenderWindow.h>

#include "inputdialog.h"
//使用的点云格式
typedef pcl::PointXYZRGBA PointTRGBA;
typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloudT;
typedef pcl::PointCloud<pcl::PointXYZRGBA> PointCloudTRGBA;

QT_BEGIN_NAMESPACE
namespace Ui {
class PCLVisualizer;
}
QT_END_NAMESPACE

class PCLVisualizer : public QMainWindow
{
  Q_OBJECT

public:
  PCLVisualizer(QWidget* parent = nullptr);
  ~PCLVisualizer();

  void createActions();  //创建动作
  void createMenus();    //创建菜单
  void createToolBars(); //创建工具栏

  //点云坐标极值
  PointT p_min, p_max;

  double maxLen;

  double getMinValue(PointT p1, PointT p2);
  double getMaxValue(PointT p1, PointT p2);

public slots:

  //新建工作台
  void newWorkStation();

  void test();
  //初始化数据
  void initPointCloud();
  //连接信号槽
  void connectSS();
  //保存文件
  void savePCDFile();
  void loadPCDFile();

  //选择需要控制的坐标轴
  void chooseAxis();
  //选择颜色模式
  void chooseColorMode();
  //增大 Point Size
  void IncPointSize();
  //减小 Point Size
  void DecPointSize();
  //添加/取消坐标轴
  void AddCoordinateSystem();

protected:
  virtual void closeEvent(QCloseEvent* event);

  int point_size;
  QColor point_color;
  //创建一个共享的PCLVisualizer 对象用于显示
  pcl::visualization::PCLVisualizer::Ptr viewer_;
  //创建一个共享指针用于保存点云
  //原始点云
  PointCloudT::Ptr cloud_;
  //彩色点云
  PointCloudTRGBA::Ptr cloudRGBA_;

  /** @brief 坐标轴：0 = x | 1 = y | 2 = z */
  int filtering_axis_;

  /** @brief 颜色模式：Holds the color mode for @ref colorCloudDistances */
  int color_mode_;

  /** @brief Color point cloud on X,Y or Z axis using a Look-Up Table (LUT)
   * Computes a LUT and color the cloud accordingly, available color palettes
   * are :
   *
   *  Values are on a scale from 0 to 255:
   *  0. Blue (= 0) -> Red (= 255), this is the default value
   *  1. Green (= 0) -> Magenta (= 255)
   *  2. White (= 0) -> Red (= 255)
   *  3. Grey (< 128) / Red (> 128)
   *  4. Blue -> Green -> Red (~ rainbow)
   *
   * @warning If there's an outlier in the data the color may seem uniform
   * because of this outlier!
   * @note A boost rounding exception error will be thrown if used with a non
   * dense point cloud
   */
  void colorCloudDistances();

private slots:

  //工具栏中的视图选择工作 6个
  void on_actionUp_triggered();

  void on_actionBottom_triggered();

  void on_actionFront_triggered();

  void on_actionBack_triggered();

  void on_actionLeft_triggered();

  void on_actionRight_triggered();

  void on_actionBGColor_triggered();

  void on_actionabout_triggered();

  void on_comboBox_Color_currentIndexChanged(const QString& arg1);

  void on_actionCoordinateSystem_triggered();

private:
  Ui::PCLVisualizer* ui;
  inputDialog* inputDlg;

  QString logStr;
  QStringList logList;

  bool isRBGA;

  QColor bgColor;

  QMenu* fileMenu; //各项菜单栏
  QMenu* zoomMenu;
  QMenu* rotateMenu;
  QMenu* mirrorMenu;

  QString fileName;

  //点云文件处理动作
  QAction* addCloudAction;
  QAction* newWorkStationAction;
  QAction* exportCloud2PCDAction;
  QAction* exportCloud2PLYAction;
  QAction* newCloudAction;
  QAction* copyCloudAction;
  QAction* cutCloudAction;
  QAction* pasteCloudAction;
  QAction* searchCloudAction;
  QAction* export2CSVAction;
  QAction* export2TXTAction;
  QAction* starCloudAction;
  QAction* exportLogAction;
  QAction* snapShotAction;

  QAction* outliersRemoveAction; //镜像菜单项
  QAction* filterAction;
  QAction* alignAction;
  QAction* MLSAction;
  QAction* downSampleAction;
  QAction* cloudSpliceAction;
  QAction* HistogramAction;
  QAction* surfaceAction;

  QToolBar* fileTool; //工具栏
  QToolBar* algorithmTool;
};
#endif // PCLVISUALIZER_H
