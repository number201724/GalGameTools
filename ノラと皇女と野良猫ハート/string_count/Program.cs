using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace string_count
{
    class Program
    {
        static void Main(string[] args)
        {
            string[] files = Directory.GetFiles("C:\\Users\\yuanrui\\Desktop\\deode\\scene");

            int string_count = 0;
            foreach(string file in files)
            {
                using (StreamReader sr = new StreamReader(file))
                {
                    while(!sr.EndOfStream)
                    {
                        string s = sr.ReadLine();
                        if (s.Length > 0)
                        {
                            string_count += s.Length;
                        }
                    }
                    
                }
            }

            Console.WriteLine(string_count);
        }
    }
}
