using System;
using System.Windows.Forms;

namespace ledRecognize.view
{
    public partial class startForm : Form
    {
        public startForm()
        {
            InitializeComponent();
        }

        public string batch;    //批次编号
        public string num;  //开始序号

        private void ok_btn_Click(object sender, EventArgs e)
        {
            //按下确定按钮，获取编号和序号
            if (textBox1.Text != "" && textBox2.Text != "")
            {
                batch = textBox1.Text;
                num = textBox2.Text;
                this.Dispose();
            }
            else
            {
                this.DialogResult = DialogResult.None;
                MessageBox.Show("请输入批次编号和开始序号!");
            }

        }

        //当按下取消按钮，释放窗口
        private void cancel_btn_Click(object sender, EventArgs e)
        {
            this.Dispose();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            
            if (textBox1.Text != "" && textBox2.Text != "")
            {
                ok_btn.Enabled = true;
            }
                
        }

        //当序号输入框输入发生改变时触发
        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            if (textBox2.Text != "")
            {
                try
                {
                    UInt32 temp = Convert.ToUInt32(textBox2.Text);
                    if (temp == 0)
                    {
                        ToolTip tooltip = new ToolTip();
                        tooltip.Show("数值超出范围", textBox2, 1000);
                        //tooltip.Show("数值超出范围", textBox2, 1000);
                    }
                }
                catch (System.OverflowException)
                {
                    ToolTip tooltip = new ToolTip();
                    tooltip.Show("数值超出范围", textBox2);
                }
               
            }
            if (textBox1.Text != "" && textBox2.Text != "")
            {
                ok_btn.Enabled = true;
            }
        }

        //限制textBox1只能输入字母、数字、下划线和减号
        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!(Char.IsLetterOrDigit(e.KeyChar)||e.KeyChar=='_'|| e.KeyChar == '-' || Char.IsControl(e.KeyChar)))
            {
                e.Handled = true;
            }
        }

        //限制textBox2只能输入数字
        private void textBox2_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar != 8 && !Char.IsDigit(e.KeyChar))
            {
                e.Handled = true;
            }
        }

    }
}
