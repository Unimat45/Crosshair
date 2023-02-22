namespace Pointer {
    using System.Data;
    using Newtonsoft.Json;
    using System.Diagnostics;

    /// <summary>
    /// Represents a configuration file
    /// </summary>
    public class Config {
        // Filename constant
        private readonly static string CONFIG_FILE = "config.json";

        // All the available configs
        private static List<Config>? configs = new();
        // Currently activated config or null if not config is loaded
        private static Config? _activeConfig;
        public static Config? ActiveConfig { get => _activeConfig; private set { _activeConfig = value; } }

        private readonly string _processName;
        private readonly string? _widthRule;
        private readonly string? _heightRule;
        private readonly int _x;
        private readonly int _y;

        private int _width;
        private int _height;

        /// <summary>
        /// Name of the process to detect
        /// </summary>
        public string ProcessName => _processName;
        /// <summary>
        /// String representation of the width rule
        /// </summary>
        public string? WidthRule => _widthRule;
        /// <summary>
        /// String representation of the height rule
        /// </summary>
        public string? HeightRule => _heightRule;
        /// <summary>
        /// Actual X position of the crossair (-1 if not used)
        /// </summary>
        public int X => _x;
        /// <summary>
        /// Actual Y position of the crossair (-1 if not used)
        /// </summary>
        public int Y => _y;

        /// <summary>
        /// Creates a new instance of config
        /// </summary>
        /// <param name="screenWidth">Width of the screen</param>
        /// <param name="screenHeight">Height of the screen</param>
        /// <param name="processName">Name of the process used ("None" for no process)</param>
        /// <param name="widthRule">Rule for the X position</param>
        /// <param name="heightRule">Rule for the Y position</param>
        public Config(int screenWidth, int screenHeight, string processName, string widthRule, string heightRule) {
            _width = screenWidth;
            _height = screenHeight;

            _processName = processName;
            _widthRule = widthRule;
            _heightRule = heightRule;
            _x = -1;
            _y = -1;
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

        /// <summary>
        /// Calculates the actual X position
        /// </summary>
        /// <returns>The actual X positiom</returns>
        public int CalculateWidth() {
            if (_widthRule != null) {
                return (int)Eval(_widthRule?.Replace("%w", _width.ToString()) ?? "0");
            }
            return _x;
        }

        /// <summary>
        /// Calculates the actual Y position
        /// </summary>
        /// <returns>The actual Y positiom</returns>
        public int CalculateHeight() {
            if (_heightRule != null) {
                return(int)Eval(_heightRule?.Replace("%h", _height.ToString()) ?? "0");
            }
            return _y;
        }

        /// <summary>
        /// Converts a literal string to a computed result
        /// </summary>
        /// <param name="expression">string representation of a mathematical operation</param>
        /// <returns>The result of the mathematical operation</returns>
        /// <exception cref="ConfigParseException"></exception>
        private static double Eval(string expression) {
            DataTable dt = new();
            object computed = dt.Compute(expression, string.Empty);

            // If error computing, throw custom error
            if (computed == DBNull.Value) {
                throw new ConfigParseException($"Invalid expression: {expression}");
            }

            return Convert.ToDouble(computed);
        }

        /// <summary>
        /// Load all configs from the config.json file
        /// </summary>
        /// <param name="width">Width of the screen</param>
        /// <param name="height">Height of the screen</param>
        public static void LoadFromFile(int width, int height) {
            string data = File.ReadAllText(CONFIG_FILE);
            configs = JsonConvert.DeserializeObject<List<Config>>(data);

            // Since screen size may have changed since last run, they are provided as parameters
            configs?.ForEach(c => { c._width = width; c._height = height; });

            // Starts the process scanning thread
            new Task(ScanForProcesses).Start();
        }

        /// <summary>
        /// Scans process and load configs if one is found
        /// </summary>
        public static void ScanForProcesses() {
            while (true) {
                // If not configs are loaded, no need to stay in the loop
                if (configs == null || configs.Count == 0) break;
                
                // If no config is loaded
                if (_activeConfig == null) {
                    foreach(Config config in configs) {
                        Process[] ps = Process.GetProcessesByName(config.ProcessName);
                        // Scan if a process with the name exists
                        if (ps.Length != 0) {
                            // Loads the config if at least one is found
                            _activeConfig = config;
                            break;
                        }
                    }
                }
                else {
                    // Check if the process still exists
                    Process[] ps = Process.GetProcessesByName(_activeConfig.ProcessName);
                    if (ps.Length == 0) {
                        // If not, unload the config
                        _activeConfig = null;
                        break;
                    }
                }
                // Sleeps for 1 sec
                Thread.Sleep(1000);
            }
        }
    }

    /// <summary>
    /// Represents an error in a config file
    /// </summary>
    public class ConfigParseException : Exception {
        public ConfigParseException(string message) : base(message) { }
    }
}
