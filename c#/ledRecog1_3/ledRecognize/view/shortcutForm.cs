using System;
using System.Windows.Forms;

using ledRecognize.dao;

namespace ledRecognize.view
{
    public partial class shortcutForm : Form
    {
        private string prefix = "";
        private string key = "";
        Button shortcut = null;

        public shortcutForm(Button capture_btn)
        {
            InitializeComponent();
            shortcut = capture_btn;

            textBox1.Text = "Alt + " + options.getShortcut();

            //textBox2.Clear();
            button1.Enabled = false;
        }

        //取消
        private void button2_Click(object sender, EventArgs e)
        {
            this.Dispose();
        }

        //确定
        private void button1_Click(object sender, EventArgs e)
        {
            //在此处完成设置快捷键
            shortcut.Text = "识别(&"+key+")";

            //能按下这个键，prefix != "" && key != ""
            options.setShortcut(prefix+key);
        }

        private void textBox2_KeyUp(object sender, KeyEventArgs e)
        {
            if (prefix != "" && key != "")
            {
                textBox2.Text = prefix + key;
                button1.Enabled = true;
            }else
            {
                textBox2.Clear();
                button1.Enabled = false;
            }

            //textBox2.Clear();
        }

        private void textBox2_KeyDown(object sender, KeyEventArgs e)
        {
            //ToolTip tooltip = new ToolTip();
            //tooltip.Show(""+e.KeyCode, textBox2,500);

            //MessageBox.Show("e.KeyCode = " + e.KeyCode);

            //textBox2.Clear();

            if (e.Alt || e.Control || e.Shift)
            {
                prefix = "Alt + ";
                textBox2.Text = prefix;
            }
            else
            {
                prefix = "";
            }

            if(e.KeyCode>=Keys.A && e.KeyCode <= Keys.Z)
            {
                key = "" + e.KeyCode;
                //textBox2.Text = prefix + key;
            }
            else
            {
                key = "";
            }

            //switch (e.KeyCode)
            //{
            //    case Keys.Control:
            //        ctrl = true;
            //        break;
            //    case Keys.Alt:
            //        alt = true;
            //        break;
            //    case Keys.Shift:
            //        shift = true;
            //        break;
            //    default:
            //        if(key=="")
            //        {
            //            key = "" + e.KeyCode;
            //        }
            //        break;
            //}


            //if (e.Control)
            //{
            //    textBox2.Text += e.KeyCode;
            //}
            //if (e.Alt)
            //{
            //    textBox2.Text += e.KeyCode;
            //}


            //if (e.KeyCode == Keys.D && e.Control)
            //{
            //    MessageBox.Show("e.keycode"+e.KeyCode);
            //}
        }

    }
}
