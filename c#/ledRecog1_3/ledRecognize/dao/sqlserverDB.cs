using System;
using System.Collections.Generic;

using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Data.SqlClient;

namespace ledRecognize.dao
{
    class sqlserverDB
    {
        //单例设计模式
        private static string mssql_con_string = "Data Source =localhost\\SQLEXPRESS; User ID = sa; Password = 1307;"
            + "Initial Catalog = recognize";//由于是express版本，需要用这个 (local)\\SQLEXPRESS 
        private static SqlConnection mssql_con = new SqlConnection(mssql_con_string);

        private sqlserverDB()
        {

        }

        /// <summary>
        /// 建立到SQL server数据库的连接
        /// </summary>
        /// <returns></returns>
        public static SqlConnection getInstance()
        {
            return mssql_con;
        }

        /// <summary>
        /// 将一条记录插入数据库，操作成功返回true，否则返回false
        /// </summary>
        /// <param name="batch_num"></param>
        /// <param name="serial_num"></param>
        /// <param name="results"></param>
        /// <param name="date_time"></param>
        /// <returns></returns>
        public static Boolean insertValues(string batch_num, Int32 serial_num, double results, DateTime date_time)
        {
            SqlConnection con = null;
            try
            {
                con = getInstance();
                con.Open();

                SqlCommand cmd = new SqlCommand("insert into records(batch_num,serial_num,results,date_time) "
                    + "values('"+ batch_num + "', '" + serial_num + "', '"+ results+"', '"
                    + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "')", con);

                if (cmd.ExecuteNonQuery() == 1)
                {
                    Console.WriteLine("数据插入成功！");
                    return true;
                }
                return false;
            }
            catch (SqlException e)
            {
                //Console.WriteLine("数据插入失败!" + e.Message);
                return false;
            }
            finally
            {
                con.Close();
            }
        }

        public static SqlDataReader getReader(string batch)
        {
            if(batch == null)
            {
                throw new NullReferenceException("batch == null");
            }

            string sql = "select * from records where batch_num like '" + batch + "';";

            return new SqlCommand(sql, getInstance()).ExecuteReader();
        }

        public static SqlDataReader getReader(DateTime datetime)
        {
            //string sql = "select * from records where date_time like '"
            //    + datetime.ToString("yyyy-MM-dd") + "%';";

            string sql = "select * from records where date_time >= '"
                + datetime.ToString("yyyy-MM-dd") + "' and date_time < '"
                + datetime.AddDays(1).ToString("yyyy-MM-dd") + "'";

            return new SqlCommand(sql, getInstance()).ExecuteReader();
        }

        internal static SqlDataReader getReader()
        {
            string sql = "select * from records";

            return new SqlCommand(sql, getInstance()).ExecuteReader();
        }
    }
}
