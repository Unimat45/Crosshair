namespace Updater {
    using System;
    using Requests;
    using System.IO;
    using System.Text;
    using System.Windows;
    using Newtonsoft.Json;
    using System.Net.Http;
    using System.Diagnostics;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application {
        protected override void OnStartup(StartupEventArgs e) {
            base.OnStartup(e);

            using HttpClient client = new();

            client.DefaultRequestHeaders.Add("User-Agent", "Crosshair-Updater");

            HttpResponse res = client.Get("https://api.github.com/repos/unimat45/crosshair/releases/latest");
            GithubRes data = GithubRes.FromJson(res.Text);

            string json_data = File.Exists("version.json") ? File.ReadAllText("version.json", Encoding.UTF8) : $"{{\"verison\": \"0\", \"arch\": \"{(Environment.Is64BitOperatingSystem ? "x64" : "x86")}\"}}";
            VersionFile file = JsonConvert.DeserializeObject<VersionFile>(json_data) ?? new();

            if (data.TagName != file.Version) {
                MainWindow m = new(data, file.Arch);
                m.ShowDialog();
                File.WriteAllText("version.json", $"{{\"version\": \"{data.TagName}\", \"arch\": \"{file.Arch}\"}}");
            }
            
            ProcessStartInfo info = new("Crosshair\\Crosshair.exe") {
                UseShellExecute = false,
                CreateNoWindow = true,
                WorkingDirectory = Path.Combine(Directory.GetCurrentDirectory(), "Crosshair")
            };

            Process.Start(info);

            Environment.Exit(0);
        }
    }
}
