namespace Updater {
    using System;
    using Requests;
    using System.IO;
    using System.Linq;
    using System.Windows;
    using System.Net.Http;
    using System.Threading;
    using System.IO.Compression;
    using System.Threading.Tasks;

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        private readonly GithubRes data;
        private readonly string arch;
        public MainWindow(GithubRes data, string arch) {
            InitializeComponent();
            this.data = data;
            this.arch = arch;
            DownloadLatestVersion();
        }

        public void DownloadLatestVersion() {
            HttpClient client = new();

            Asset asset = data.Assets?.FirstOrDefault(a => a.Name!.Contains(arch)) ?? data.Assets!.First();
            this.TxtLabel.Text = $"Updating to version {data.TagName}...";

            string tmpPath = Path.GetTempFileName();

            IProgress<float> prg = new Progress<float>(f => this.PrgBar.Value = f * .9f);

            new Task(async () => {
                FileStream fs = new(tmpPath, FileMode.Create, FileAccess.Write, FileShare.None);
                await client.DownloadAsync(asset.BrowserDownloadUrl.OriginalString, fs, prg);
                fs.Close();
                client.Dispose();

                ZipFile.ExtractToDirectory(tmpPath, Directory.GetCurrentDirectory(), true);

                File.Delete(tmpPath);

                this.PrgBar.Dispatcher.Invoke(() => { this.PrgBar.Value = 1; });

                Thread.Sleep(2000); // 2 secs

                this.Dispatcher.Invoke(() => { this.Close(); });
            }).Start();
        }
    }
}
