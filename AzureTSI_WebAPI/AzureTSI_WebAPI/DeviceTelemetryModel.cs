using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace AzureTSI_WebAPI
{
    public class DeviceTelemetryModel
    {
        [JsonProperty("timestamps")]
        public List<string> Timestamp { get; set; }
        public List<PropertyModel> Properties { get; set; }
    }

    public class PropertyModel
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("type")]
        public string Type { get; set; }

        [JsonProperty("values")]
        public List<JValue> Values { get; set; }
    }
}
