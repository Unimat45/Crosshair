
namespace Updater {
    using Newtonsoft.Json;
    using Newtonsoft.Json.Converters;
    public class VersionFile {
        [JsonProperty("version")]
        public string Version { get; set; }

        [JsonProperty("arch")]
        public string Arch { get; set; }
    }
}
