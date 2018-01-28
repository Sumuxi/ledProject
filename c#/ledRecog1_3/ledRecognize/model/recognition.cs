using System;

using System.Runtime.InteropServices;// 加上这一句才可以调用 .dll 文件

namespace ledRecognize.model
{
    //这个类中调用 dll 中的函数来识别数字，返回识别结果（一个字符串）
    //string result = Marshal.PtrToStringAnsi(recognize())
    class recognition
    {
        //  begin  
        [DllImport("recog1_6.dll", EntryPoint = "recognize")]
        public static extern string recognize(string filename);//char * recognize(char * filename){}
        //end 

        //public static string getResult(string filename)
        //{

        //}
    }
}
