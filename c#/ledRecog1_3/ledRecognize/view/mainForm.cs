using System;
using System.Windows.Forms;
using Emgu.CV;
using Emgu.CV.Structure;

using ledRecognize.dao;
using ledRecognize.model;
using System.IO;

namespace ledRecognize.view
{
    public partial class mainForm : Form
    {
        private string batch = null;//批号
        private Int32 serial = 0;//序号
        private bool flag = false;//是否有excel文件需要保存？

        private VideoCapture _capture = null;
        private Mat frame = null;

        public mainForm()
        {
            InitializeComponent();
            CvInvoke.UseOpenCL = false;
        }

        //循环获取一帧图像
        private void ProcessFrame(object sender, EventArgs arg)
        {
            if (_capture != null && _capture.Ptr != IntPtr.Zero)
            {
                _capture.Retrieve(frame, 0);
                imageBox1.Image = frame;
            }
        }

        /// <summary>
        /// 打开摄像头
        /// </summary>
        private void startCamera()
        {
            try
            {
                //打开本机默认摄像头，参数默认空或0，也可以填入1或其他表示其他摄像头
                _capture = new VideoCapture();
                _capture.ImageGrabbed += ProcessFrame;
            }
            catch (NullReferenceException excpt)
            {
                MessageBox.Show(excpt.Message);
            }
            frame = new Mat();
            _capture.Start();
        }

        /// <summary>
        /// 主窗口加载时要完成的动作
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Form1_Load(object sender, EventArgs e)
        {
            //容器窗口不可见
            splitContainer1.Visible = false;
            ledRecognize.dao.Export.loadExcel();

            string key = options.getShortcut();
            capture_btn.Text = "识别(&" + key+ ")";

        }

        /// <summary>
        /// 新批次
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void new_Item_Click(object sender, EventArgs e)
        {
            startForm start = new startForm();
            //启动一个模态对话框
            start.ShowDialog();
            //MessageBox.Show(start.batch+"  "+ start.num);
            //仅当对话框返回OK时，才可赋值
            if(start.DialogResult == DialogResult.OK)
            {
                this.batch = start.batch;
                this.serial = Convert.ToInt32(start.num);
                textBox1.Text = start.batch;
                textBox2.Text = start.num;

                if (!splitContainer1.Visible)//下面的代码只会执行一次
                {
                    splitContainer1.Visible = true;
                    startCamera();
                }
            }
            
        }

        /// <summary>
        /// 继续上一批次
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void continue_Item_Click(object sender, EventArgs e)
        {
            //具有两个返回的值的方法
            options.getRecent(out batch, out serial);

            textBox1.Text = batch;
            textBox2.Text = serial.ToString();

            if (!splitContainer1.Visible)//下面的代码只会执行一次
            {
                splitContainer1.Visible = true;
                startCamera();
            }
        }

        /// <summary>
        /// 拍照捕获，识别
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void capture_btn_Click(object sender, EventArgs e)
        {
            if (batch == null || serial == 0)
                return;

            string path = ".\\pictures\\"+DateTime.Now.ToString("yyyy-MM-dd")+"\\" +batch;
            if (!Directory.Exists(path))
            {
                Directory.CreateDirectory(path);
            }
            _capture.Retrieve(frame, 0);//从摄像头读取一帧图像
            imageBox2.Image = frame;//显示图像
            Image<Bgr, byte> img = frame.ToImage<Bgr, byte>();
            string filename = path + "\\" + serial.ToString() + ".jpg";
            img.Save(filename);

            //调用图像识别函数，返回结果到result
            string result = recognition.recognize(filename);
            if (result == null && result.Length == 0)
            {
                MessageBox.Show("未检测到数字，请重试！");
                if(File.Exists(filename))
                    File.Delete(filename);
                return;
            }
            try
            {
                Convert.ToDouble(result);
            }
            catch (FormatException )
            {
                MessageBox.Show("抱歉！无法识别。");
                if (File.Exists(filename))
                    File.Delete(filename);
                return;
            }
            catch (OverflowException )
            {
                MessageBox.Show("抱歉！无法识别。");
                if (File.Exists(filename))
                    File.Delete(filename);
                return;
            }

            string[] values = new string[3];
            values[0] = batch;
            values[1] = serial.ToString();
            values[2] = result;
            dataGridView1.Rows.Add(values);
            dataGridView1.CurrentCell = dataGridView1.Rows[dataGridView1.RowCount - 1].Cells[0];
            dataGridView1.FirstDisplayedScrollingRowIndex = dataGridView1.RowCount - 1;

            //将一次记录写入excel表格
            ledRecognize.dao.Export.writeToExcel(batch, serial, result);
            flag = true;

            serial++;//serial始终指向下一次开始的序号
            textBox2.Text = "" + serial;
            textBox3.Text = result;
        }

        /// <summary>
        /// 设置快捷键
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void shortcut_Item_Click(object sender, EventArgs e)
        {
            
            Form shortcut = new shortcutForm(capture_btn);
            shortcut.ShowDialog();
        }

        /// <summary>
        /// 主窗口关闭
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (_capture != null)
            {
                _capture.Dispose();
            }

            if (flag&& MessageBox.Show("是否保存记录文件？", "文件未保存", MessageBoxButtons.OKCancel) == DialogResult.OK) {

                SaveFileDialog sfg = new SaveFileDialog();
                sfg.Title = "请选择保存文件的路径";
                sfg.InitialDirectory = options.getPath();
                sfg.Filter = "Excel 工作簿|*.xls";
                //sfg.FileName = DateTime.Now.ToShortDateString().Replace('/', '-');
                //string filename = "d:\\records-"+batch+"-"+ DateTime.Now.ToString("MM-dd HH-mm-ss") +".xls";
                sfg.OverwritePrompt = true;
                if(sfg.ShowDialog() == DialogResult.OK)//点击保存
                {
                    string filename = sfg.FileName;
                    ledRecognize.dao.Export.saveExcel(filename);//保存退出
                }
                else//点击取消
                {
                    ledRecognize.dao.Export.exitExcel();//强制退出
                }
                
            }
            else
            {
                ledRecognize.dao.Export.exitExcel();//强制退出
            }

            //将批号和序号写入setting.xml文件中
            if (batch == null || serial == 0)
                return;
            options.setRecent(batch, serial);
        }

        private void save_btn_Click(object sender, EventArgs e)
        {
            if (flag == false)//没有新纪录
            {
                MessageBox.Show("不存在有效数据！");
                return;
            }

            SaveFileDialog sfg = new SaveFileDialog();
            sfg.Title = "请选择保存文件的路径";
            sfg.InitialDirectory = options.getPath();
            sfg.Filter = "Excel 工作簿|*.xls";
            //sfg.FileName = DateTime.Now.ToShortDateString().Replace('/', '-');
            //string filename = "d:\\records-"+batch+"-"+ DateTime.Now.ToString("MM-dd HH-mm-ss") +".xls";
            sfg.OverwritePrompt = true;
            if (sfg.ShowDialog() == DialogResult.OK)//点击保存
            {
                string filename = sfg.FileName;
                ledRecognize.dao.Export.saveExcel(filename);//保存退出
                ledRecognize.dao.Export.loadExcel();//新加载一个excel文件
                flag = false;
            }

        }
    }
}
