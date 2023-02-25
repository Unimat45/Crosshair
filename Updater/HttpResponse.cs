using HttpStatusCode = System.Net.HttpStatusCode;
using Encoding = System.Text.Encoding;
using JsonSerializer = System.Text.Json.JsonSerializer;

namespace Requests {
    public class HttpResponse {
        private readonly string raw;
        private readonly byte[] raw_bytes;
        private readonly HttpStatusCode status;

        internal HttpResponse(string raw, HttpStatusCode status) {
            this.raw = raw;
            this.raw_bytes = Encoding.UTF8.GetBytes(raw);
            this.status = status;
        }

        internal HttpResponse(byte[] raw_bytes, HttpStatusCode status) {
            this.raw_bytes = raw_bytes;
            this.raw = Encoding.UTF8.GetString(raw_bytes);
            this.status = status;
        }

        public override string ToString() => this.raw;
        public string Text => this.ToString();
        public byte[] Content => this.raw_bytes;
        public HttpStatusCode StatusCode => this.status;
        public bool IsOK => this.status == HttpStatusCode.OK;

        public T ToJSON<T>() {
            return JsonSerializer.Deserialize<T>(this.raw)!;
        }

    }
}
