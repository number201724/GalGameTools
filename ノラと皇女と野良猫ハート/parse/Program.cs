using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace parse
{
    class Program
    {
        static void Main(string[] args)
        {
            if(args.Length <= 0)
            {
                Console.WriteLine("请输入文件名来格式化文本");
                return;
            }
            string filename = args[0].Substring(args[0].LastIndexOf('\\') + 1);

            Directory.CreateDirectory("C:\\Users\\yuanrui\\Documents\\GalGameTools\\ノラと皇女と野良猫ハート\\scene");
            string line;
            using (StreamWriter sw = new StreamWriter(File.Create("C:\\Users\\yuanrui\\Documents\\GalGameTools\\ノラと皇女と野良猫ハート\\scene/" + filename), Encoding.Unicode))
            {
                using (StreamReader sr = new StreamReader(File.Open(args[0], FileMode.Open), Encoding.GetEncoding("SHIFT-JIS")))
                {
                    while (!sr.EndOfStream)
                    {
                        line = sr.ReadLine();

                        if (line.Length > 0 && line[0] > 0x80)
                        {
                            string text = line;
                            int i = line.IndexOf("】");
                            if(i != -1)
                            {
                                text = line.Substring(i + 1);
                            }
                            sw.WriteLine("//"+line);
                            sw.WriteLine("☆" + line);
                            sw.WriteLine("");
                        }
                    }
                }
            }
           
        }
    }
}
