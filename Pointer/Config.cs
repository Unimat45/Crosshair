namespace Pointer {
    using System.Data;
    using System.Diagnostics;
    using Newtonsoft.Json;

    public class Config {
#if DEBUG
        private readonly static string CONFIG_FILE = @"C:\Users\Mathieu\Desktop\Pointer\Pointer\config.json";
#else
        private readonly static string CONFIG_FILE = "config.json";
#endif
        private static List<Config>? configs = new();
        private static Config? _activeConfig;
        public static Config? ActiveConfig { get => _activeConfig; private set { _activeConfig = value; } }

        private readonly string _processName;
        private readonly string? _widthRule;
        private readonly string? _heightRule;
        private readonly int _x;
        private readonly int _y;

        private int _width;
        private int _height;

        public string ProcessName => _processName;
        public string? WidthRule => _widthRule;
        public string? HeightRule => _heightRule;
        public int X => _x;
        public int Y => _y;

        public Config(int screenWidth, int screenHeight, string processName, string widthRule, string heightRule) {
            _width = screenWidth;
            _height = screenHeight;

            _processName = processName;
            _widthRule = widthRule;
            _heightRule = heightRule;
            _x = -1;
            _y = -1;
        }

        public Config(string processName, int x, int y) {
            _width = -1;
            _height = -1;

            _processName = processName;
            _x = x;
            _y = y;
        }

        [JsonConstructor]
        private Config(string processName, string? widthRule, string? heightRule, int x, int y, int width, int height) {
            _processName = processName;
            _widthRule = widthRule;
            _heightRule = heightRule;
            _x = x;
            _y = y;
            _width = width;
            _height = height;
        }

        public void SaveToConf() {
            string data = JsonConvert.SerializeObject(this);
            File.WriteAllText("test.json", data);
        }
        public int CalculateWidthRule() => (int)Eval(_widthRule?.Replace("%w", _width.ToString()) ?? "0");
        public int CalculateHeightRule() => (int)Eval(_heightRule?.Replace("%h", _height.ToString()) ?? "0");

        private static double Eval(string expression) {
            DataTable dt = new();
            return Convert.ToDouble(dt.Compute(expression, string.Empty));
        }

        public static void LoadFromFile(int width, int height) {
            string data = File.ReadAllText(CONFIG_FILE);
            configs = JsonConvert.DeserializeObject<List<Config>>(data);

            configs?.ForEach(c => { c._width = width; c._height = height; });

            new Task(ScanForProcesses).Start();
        }

        public static void ScanForProcesses() {
            while (true) {
                if (configs != null && configs.Count != 0) {
                    if (_activeConfig == null) {
                        foreach(Config config in configs) {
                            Process[] ps = Process.GetProcessesByName(config.ProcessName);
                            if (ps.Length != 0) {
                                _activeConfig = config;
                                break;
                            }
                        }

                    }
                    else {
                        Process[] ps = Process.GetProcessesByName(_activeConfig.ProcessName);
                        if (ps.Length == 0) {
                            _activeConfig = null;
                            break;
                        }
                    }
                }
                Thread.Sleep(1000);
            }
        }
    }
}
