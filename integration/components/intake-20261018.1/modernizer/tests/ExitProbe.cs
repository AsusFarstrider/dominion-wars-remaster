// Test process only: no windows, graphics, game data, or display changes.
public class ExitProbe
{
    public static int Main()
    {
        var output = System.Environment.GetEnvironmentVariable("DW_TEST_FOCUS_ENVIRONMENT_OUTPUT");
        if (!System.String.IsNullOrEmpty(output))
        {
            System.IO.File.WriteAllText(output,
                System.Environment.GetEnvironmentVariable("DW_EXPERIMENTAL_FOCUS_GUARD") ?? "<unset>");
        }
        return 37;
    }
}
