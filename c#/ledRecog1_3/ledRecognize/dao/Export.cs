using System;
using System.Reflection; // 引用这个才能使用Missing字段 
using Excel = Microsoft.Office.Interop.Excel;//导入的是Microsoft Office 15.0 Object Library，支持的文件格式是.xlsx
using System.Data.SqlClient;

namespace ledRecognize.dao
{
    /// <summary>
    /// 用于编辑excel文件
    /// </summary>
    class Export
    {
        private static Excel.Application xApp;
        private static Excel.Workbook xBook;
        private static Excel.Worksheet xSheet;
        private static int i = 1;//开始写的行数

        /// <summary>
        /// 在内存中加载一个excel文件
        /// </summary>
        public static void loadExcel()//静态构造函数，在创建第一个实例或引用任何静态成员之前，将自动调用静态构造函数来初始化类。
        {
            xApp = new Excel.Application();
            xApp.Visible = false;
            xBook = xApp.Workbooks.Add(Missing.Value);//新建一个excel文件的代码
            //指定要操作的Sheet，这是一种方式：
            xSheet = xBook.Sheets[1];
            Excel.Range allColumn = xSheet.Columns;
            allColumn.EntireRow.AutoFit();
            allColumn.EntireColumn.AutoFit();

            //写入表头
            Excel.Range a1 = xSheet.get_Range("A1", Missing.Value); //C6单元格
            a1.Value2 = "批次编号";
            Excel.Range b1 = xSheet.get_Range("B1", Missing.Value); //C6单元格
            b1.Value2 = "序号";
            Excel.Range c1 = xSheet.get_Range("C1", Missing.Value); //C6单元格
            c1.Value2 = "记录值";
            Excel.Range d1 = xSheet.get_Range("D1", Missing.Value); //C6单元格
            d1.Value2 = "记录时间";
        }
        
        /// <summary>
        /// 将一行记录写入excel文件
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="serial"></param>
        /// <param name="result"></param>
        public static void writeToExcel(string batch, int serial, string result)
        {
            i++;
            Excel.Range rng1 = (Excel.Range)xSheet.Cells[i, 1];
            rng1.Value2 = batch;
            Excel.Range rng2 = (Excel.Range)xSheet.Cells[i, 2];
            rng2.Value2 = serial;
            Excel.Range rng3 = (Excel.Range)xSheet.Cells[i, 3];
            rng3.Value2 = Convert.ToDouble(result);
            Excel.Range rng4 = (Excel.Range)xSheet.Cells[i, 4];
            rng4.Value2 = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
        }

        /// <summary>
        /// 保存excel文件并退出
        /// </summary>
        /// <param name="filename"></param>
        public static void saveExcel(string filename)
        {
            int FormatNum;//保存excel文件的格式
            string Version;//excel版本号
            Version = xApp.Version;//获取你使用的excel 的版本号
            if (Convert.ToDouble(Version) < 12)//You use Excel 97-2003
            {
                FormatNum = -4143;
            }
            else//you use excel 2007 or later
            {
                FormatNum = 56;
            }
            //workbook.SaveAs(@".\MyExcel.xls", FormatNum);
            //关闭EXCEL的提示框
            xApp.DisplayAlerts = false;
            xBook.SaveAs(filename, FormatNum);

            xSheet = null;
            xBook = null;
            xApp.Quit();    //这一句非常重要，否则Excel对象不能从内存中退出
            xApp = null;
        }

        /// <summary>
        /// 强制退出excel文件
        /// </summary>
        public static void exitExcel()
        {
            xBook.Saved = true;//这句话一定要，以让excel进程退出
            xSheet = null;
            xBook = null;
            xApp.Quit();    //这一句非常重要，否则Excel对象不能从内存中退出
            xApp = null;
        }

        /// <summary>
        /// 按批次编号查询数据库，将得到的数据写入excel文件
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="filename"></param>
        /// <returns></returns>
        public static Boolean toExcel(string batch, string filename)
        {
            //创建Applicaton对象
            Excel.Application xApp = new Excel.Application();

            xApp.Visible = false;

            Excel.Workbook xBook = xApp.Workbooks.Add(Missing.Value); //新建一个excel文件的代码

            //指定要操作的Sheet，两种方式：
            Excel.Worksheet xSheet = xBook.Sheets[1];
            //Excel.Worksheet xSheet = xApp.ActiveSheet;

            Excel.Range allColumn = xSheet.Columns;
            allColumn.EntireRow.AutoFit();
            allColumn.EntireColumn.AutoFit();

            //写入数据，将数据写入到指定的单元格
            Excel.Range a1 = xSheet.get_Range("A1", Missing.Value); //C6单元格
            a1.Value2 = "批次编号";
            Excel.Range b1 = xSheet.get_Range("B1", Missing.Value); //C6单元格
            b1.Value2 = "序号";
            Excel.Range c1 = xSheet.get_Range("C1", Missing.Value); //C6单元格
            c1.Value2 = "记录值";
            Excel.Range d1 = xSheet.get_Range("D1", Missing.Value); //C6单元格
            d1.Value2 = "记录时间";

            SqlConnection con = sqlserverDB.getInstance();
            con.Open();
            SqlDataReader reader = sqlserverDB.getReader(batch);
            try
            {
                int i = 1;
                while (reader.Read())
                {
                    i++;
                    if (reader.HasRows)
                    {
                        Excel.Range rng1 = (Excel.Range)xSheet.Cells[i, 1];
                        rng1.Value2 = reader.GetString(0);
                        Excel.Range rng2 = (Excel.Range)xSheet.Cells[i, 2];
                        rng2.Value2 = reader.GetInt32(1).ToString();
                        Excel.Range rng3 = (Excel.Range)xSheet.Cells[i, 3];
                        rng3.Value2 = reader.GetDecimal(2).ToString();
                        Excel.Range rng4 = (Excel.Range)xSheet.Cells[i, 4];
                        rng4.Value2 = reader.GetDateTime(3).ToString("yyyy-MM-dd HH:mm:ss");
                    }
                }

                int FormatNum;//保存excel文件的格式
                string Version;//excel版本号
                //Excel.Application Application = new Excel.Application();
                //Excel.Workbook workbook = (Excel.Workbook)Application.Workbooks.Add(Missing.Value);//激活工作簿
                //Excel.Worksheet worksheet = (Excel.Worksheet)workbook.Worksheets.Add(true);//给工作簿添加一个sheet
                Version = xApp.Version;//获取你使用的excel 的版本号
                if (Convert.ToDouble(Version) < 12)//You use Excel 97-2003
                {
                    FormatNum = -4143;
                }
                else//you use excel 2007 or later
                {
                    FormatNum = 56;
                }
                //workbook.SaveAs(@"D:\MyExcel.xls", FormatNum);

                //自动调整列宽
                a1.EntireColumn.AutoFit();
                b1.EntireColumn.AutoFit();
                c1.EntireColumn.AutoFit();
                d1.EntireColumn.AutoFit();

                //保存方式二：保存WorkSheet,关闭覆盖提示
                xApp.AlertBeforeOverwriting = false;
                xBook.SaveAs(@"D:\MyExcel.xls", FormatNum);
                //xSheet.SaveAs(filename, FormatNum, Missing.Value,
                //                Missing.Value, Missing.Value, Missing.Value, Missing.Value,
                //                Missing.Value, Missing.Value, Missing.Value);
                xApp.AlertBeforeOverwriting = true;
            }
            catch (SqlException)
            {
                return false;
            }
            finally
            {
                reader.Close();
                con.Close();
                xSheet = null;
                xBook = null;
                xApp.Quit();    //这一句非常重要，否则Excel对象不能从内存中退出
                xApp = null;
            }
            return true;
        }

        /// <summary>
        /// 按记录日期查询数据库，将得到的数据写入excel文件
        /// </summary>
        /// <param name="datetime"></param>
        /// <param name="filename"></param>
        /// <returns></returns>
        public static Boolean toExcel(DateTime datetime, string filename)
        {
            //创建Applicaton对象
            Excel.Application xApp = new Excel.Application();

            xApp.Visible = false;

            Excel.Workbook xBook = xApp.Workbooks.Add(Missing.Value); //新建一个excel文件的代码

            //指定要操作的Sheet，两种方式：
            Excel.Worksheet xSheet = xBook.Sheets[1];
            //Excel.Worksheet xSheet = xApp.ActiveSheet;

            //写入数据，将数据写入到指定的单元格
            Excel.Range a1 = xSheet.get_Range("A1", Missing.Value); //C6单元格
            a1.Value2 = "批次编号";
            //rng3.Interior.ColorIndex = 6; //设备Range的背景色

            Excel.Range b1 = xSheet.get_Range("B1", Missing.Value); //C6单元格
            b1.Value2 = "序号";
            Excel.Range c1 = xSheet.get_Range("C1", Missing.Value); //C6单元格
            c1.Value2 = "记录值";
            Excel.Range d1 = xSheet.get_Range("D1", Missing.Value); //C6单元格
            d1.Value2 = "记录时间";

            //MySqlConnection mysqlCon = mysqlDB.getMysqlCon();
            //mysqlCon.Open();

            //MySqlDataReader reader = mysqlDB.getReader(datetime);

            SqlConnection con = sqlserverDB.getInstance();
            con.Open();
            SqlDataReader reader = sqlserverDB.getReader(datetime);
            try
            {
                int i = 1;
                while (reader.Read())
                {
                    i++;
                    if (reader.HasRows)
                    {
                        Excel.Range rng1 = (Excel.Range)xSheet.Cells[i, 1];
                        rng1.Value2 = reader.GetString(0);
                        Excel.Range rng2 = (Excel.Range)xSheet.Cells[i, 2];
                        rng2.Value2 = reader.GetInt32(1).ToString();
                        Excel.Range rng3 = (Excel.Range)xSheet.Cells[i, 3];
                        rng3.Value2 = reader.GetDecimal(2).ToString();
                        Excel.Range rng4 = (Excel.Range)xSheet.Cells[i, 4];
                        rng4.Value2 = reader.GetDateTime(3).ToString("yyyy-MM-dd HH:mm:ss");
                    }
                }

                int FormatNum;//保存excel文件的格式
                string Version;//excel版本号
                //Excel.Application Application = new Excel.Application();
                //Excel.Workbook workbook = (Excel.Workbook)Application.Workbooks.Add(Missing.Value);//激活工作簿
                //Excel.Worksheet worksheet = (Excel.Worksheet)workbook.Worksheets.Add(true);//给工作簿添加一个sheet
                Version = xApp.Version;//获取你使用的excel 的版本号
                if (Convert.ToDouble(Version) < 12)//You use Excel 97-2003
                {
                    FormatNum = -4143;
                }
                else//you use excel 2007 or later
                {
                    FormatNum = 56;
                }

                //自动调整列宽
                a1.EntireColumn.AutoFit();
                b1.EntireColumn.AutoFit();
                c1.EntireColumn.AutoFit();
                d1.EntireColumn.AutoFit();

                //保存方式二：保存WorkSheet
                //xApp.DisplayAlerts = false;
                xApp.AlertBeforeOverwriting = false;
                xSheet.SaveAs(filename, FormatNum);
                //xSheet.SaveAs(filename, Missing.Value, Missing.Value,
                //                Missing.Value, Missing.Value, Missing.Value, Missing.Value,
                //                Missing.Value, Missing.Value, Missing.Value);
                xApp.AlertBeforeOverwriting = true;
            }
            catch (SqlException)
            {
                return false;
            }
            finally
            {
                reader.Close();
                con.Close();
                xSheet = null;
                xBook = null;
                xApp.Quit();    //这一句非常重要，否则Excel对象不能从内存中退出
                xApp = null;
            }
            return true;
        }

        /// <summary>
        /// 查询数据库中所有数据，写入excel文件
        /// </summary>
        /// <param name="datetime"></param>
        /// <param name="filename"></param>
        /// <returns></returns>
        public static Boolean toExcel(string filename)
        {
            //创建Applicaton对象
            Excel.Application xApp = new Excel.Application();

            xApp.Visible = false;

            Excel.Workbook xBook = xApp.Workbooks.Add(Missing.Value); //新建一个excel文件的代码

            //指定要操作的Sheet，两种方式：
            Excel.Worksheet xSheet = xBook.Sheets[1];
            //Excel.Worksheet xSheet = xApp.ActiveSheet;
            
            //写入数据，将数据写入到指定的单元格
            Excel.Range a1 = xSheet.get_Range("A1", Missing.Value); //C6单元格
            a1.Value2 = "批次编号";
            //rng3.Interior.ColorIndex = 6; //设备Range的背景色

            Excel.Range b1 = xSheet.get_Range("B1", Missing.Value); //C6单元格
            b1.Value2 = "序号";
            Excel.Range c1 = xSheet.get_Range("C1", Missing.Value); //C6单元格
            c1.Value2 = "记录值";
            Excel.Range d1 = xSheet.get_Range("D1", Missing.Value); //C6单元格
            d1.Value2 = "记录时间";

            //MySqlConnection mysqlCon = mysqlDB.getMysqlCon();
            //mysqlCon.Open();

            //MySqlDataReader reader = mysqlDB.getReader(datetime);

            SqlConnection con = sqlserverDB.getInstance();
            con.Open();
            SqlDataReader reader = sqlserverDB.getReader();
            try
            {
                int i = 1;
                while (reader.Read())
                {
                    i++;
                    if (reader.HasRows)
                    {
                        Excel.Range rng1 = (Excel.Range)xSheet.Cells[i, 1];
                        rng1.Value2 = reader.GetString(0);
                        Excel.Range rng2 = (Excel.Range)xSheet.Cells[i, 2];
                        rng2.Value2 = reader.GetInt32(1).ToString();
                        Excel.Range rng3 = (Excel.Range)xSheet.Cells[i, 3];
                        rng3.Value2 = reader.GetDecimal(2).ToString();
                        Excel.Range rng4 = (Excel.Range)xSheet.Cells[i, 4];
                        rng4.Value2 = reader.GetDateTime(3).ToString("yyyy-MM-dd HH:mm:ss");
                    }
                }

                int FormatNum;//保存excel文件的格式
                string Version;//excel版本号
                //Excel.Application Application = new Excel.Application();
                //Excel.Workbook workbook = (Excel.Workbook)Application.Workbooks.Add(Missing.Value);//激活工作簿
                //Excel.Worksheet worksheet = (Excel.Worksheet)workbook.Worksheets.Add(true);//给工作簿添加一个sheet
                Version = xApp.Version;//获取你使用的excel 的版本号
                if (Convert.ToDouble(Version) < 12)//You use Excel 97-2003
                {
                    FormatNum = -4143;
                }
                else//you use excel 2007 or later
                {
                    FormatNum = 56;
                }

                //自动调整列宽
                a1.EntireColumn.AutoFit();
                b1.EntireColumn.AutoFit();
                c1.EntireColumn.AutoFit();
                d1.EntireColumn.AutoFit();

                //保存方式二：保存WorkSheet
                //xApp.DisplayAlerts = false;
                xApp.AlertBeforeOverwriting = false;
                xSheet.SaveAs(filename, FormatNum);
                //xSheet.SaveAs(filename, Missing.Value, Missing.Value,
                //                Missing.Value, Missing.Value, Missing.Value, Missing.Value,
                //                Missing.Value, Missing.Value, Missing.Value);
                xApp.AlertBeforeOverwriting = true;
            }
            catch (SqlException)
            {
                return false;
            }
            finally
            {
                reader.Close();
                con.Close();
                xSheet = null;
                xBook = null;
                xApp.Quit();    //这一句非常重要，否则Excel对象不能从内存中退出
                xApp = null;
            }
            return true;
        }

    }
}
