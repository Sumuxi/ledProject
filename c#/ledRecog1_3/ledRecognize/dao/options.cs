using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Xml.Linq;//使用XML的命名空间

namespace ledRecognize.dao
{
    //操作XML配置文件的类
    class options
    {
        //单例设计模式
        //可能抛出 System.IO.FileNotFoundException
        //private static XDocument document = XDocument.Load(@".\config\setting.xml");//加载配置文件
        private static XElement root = XDocument.Load(@".\config\setting.xml").Root;//获取到XML的根元素进行操作

        private options()
        {

        }

        public static XElement getRoot()
        {
            return root;
        }

        /// <summary>
        /// 保存批次编号和序号
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="serial"></param>
        public static void setRecent(string batch, Int32 serial)
        {
            if (batch == null)
                throw new NullReferenceException("batch==null");

            XElement recent = root.Element("recent");
            recent.SetElementValue("batch", batch);
            recent.SetElementValue("serial", serial);
            saveXML();
        }

        /// <summary>
        /// 获取上一次的批号和序号
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="serial"></param>
        public static void getRecent(out string batch, out Int32 serial)
        {
            XElement recent = root.Element("recent");
            batch = recent.Element("batch").Value;
            serial = Convert.ToInt32(recent.Element("serial").Value);
        }

        /// <summary>
        /// 设置保存路径
        /// </summary>
        /// <param name="path"></param>
        public static void setPath(string path)
        {
            if (path == null)
                throw new NullReferenceException("path==null");

            root.Element("path").SetValue(path);
            saveXML();
        }

        /// <summary>
        /// 获取上次的保存路径
        /// </summary>
        /// <returns></returns>
        public static string getPath()
        {
            return root.Element("path").Value;
        }

        /// <summary>
        /// 设置快键键
        /// </summary>
        /// <param name="shortcut"></param>
        public static void setShortcut(string shortcut)
        {
            if (shortcut == null)
                throw new NullReferenceException("key==null");

            root.Element("shortcut").SetValue(shortcut);
            saveXML();
        }

        /// <summary>
        /// return key only
        /// </summary>
        /// <returns> string key</returns>
        public static string getShortcut()
        {
            string value = root.Element("shortcut").Value;
            return value.Substring(value.Length-1,1);
        }

        public static void saveXML()
        {
            root.Save(@".\config\setting.xml");
            return;
        }
    }
}
