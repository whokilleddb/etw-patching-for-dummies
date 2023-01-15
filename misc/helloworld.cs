// Compile with: csc /target:library /out:helloworld.dll helloworld.cs
using System.Windows.Forms;

namespace HelloWorld
{
    public class Program
    {
        public static int EntryPoint(string arguments)
        {
            MessageBox.Show(arguments);
            return 0;
        }
        
        public static void Main()
        {

        }
    }
}