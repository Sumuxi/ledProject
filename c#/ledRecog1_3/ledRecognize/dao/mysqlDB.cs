using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MySql.Data.MySqlClient;

namespace ledRecognize
{
    class mysqlDB
    {
        private static string M_str_sqlcon = @"server=localhost;User Id=root;password=1307;Database=recognize"; //根据需要再设置
        private static MySqlConnection mysqlConnection = new MySqlConnection(M_str_sqlcon);

        private mysqlDB()
        {

        }

        /// <summary>
        /// 建立数据库连接.
        /// </summary>
        /// <returns>返回MySqlConnection对象</returns>
        public static MySqlConnection getMysqlCon()
        {
            return mysqlConnection;
        }

        public static MySqlDataReader getReader(string batch)
        {
            string sql = "select * from records where batch_num like '"+ batch + "';";

            MySqlCommand mysqlcommand = new MySqlCommand(sql, getMysqlCon());
            MySqlDataReader reader = mysqlcommand.ExecuteReader();
            return reader;
        }

        public static MySqlDataReader getReader(DateTime datetime)
        {
            string sql = "select * from records where date_time like '"
                + datetime.ToString("yyyy-MM-dd") + "%';";

            return new MySqlCommand(sql, getMysqlCon()).ExecuteReader();
        }

        /// <summary>
        /// 将一条记录插入数据库，操作成功返回true，否则返回false
        /// </summary>
        /// <param name="batch_num"></param>
        /// <param name="serial_num"></param>
        /// <param name="results"></param>
        /// <param name="date_time"></param>
        /// <returns></returns>
        public static Boolean insertValues(string batch_num,UInt32 serial_num,double results,DateTime date_time)
        {
            MySqlConnection mysqlCon = null;
            try
            {
                mysqlCon = getMysqlCon();
                mysqlCon.Open();//操作之前必须先打开数据库，操作完毕后要关闭

                MySqlCommand cmd =
                    new MySqlCommand(@"insert into records(batch_num,serial_num,results,date_time)"
                        + " values('" + batch_num + "','" + serial_num + "','" + results + "','"
                        + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "')", mysqlCon);

                if (cmd.ExecuteNonQuery() == 1)
                {
                    Console.WriteLine("数据插入成功！");
                    return true;
                }
                return false;
            }
            catch (MySqlException e)
            {
                //Console.WriteLine("数据插入失败!" + e.Message);
                return false;
            }
            finally
            {
                mysqlCon.Close();
            }
        }
        
    }
}
