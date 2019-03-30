using System;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.IdentityModel.Clients.ActiveDirectory;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.IO;
using System.Net;

namespace AzureTSI_WebAPI.Controllers
{
    [Route("api/[controller]")]
    public class ValuesController : Controller
    {
        private static string ApplicationClientId = "xxxxxx-xxxxx-xxx-xxxxx-xxxxxxxxxxxxx";

        // SET the application key of the application registered in your Azure Active Directory
        private static string ApplicationClientSecret = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=";

        // SET the Azure Active Directory tenant.
        private static string Tenant = "xxxxxxxxxx.onmicrosoft.com";

        //SET the TSI environment.
        private static string environmentFqdn = "xxxxxxxxxxxxxxxxxx.env.timeseries.azure.com";

        private const string TSI_DATE_FORMAT = "yyyy-MM-ddTHH:mm:ssZ";
        private const string AGGREGATE_SERIES = "aggregateSeries";
        private const string SEARCH_SPAN_KEY = "searchSpan";
        private const string FROM_KEY = "from";
        private const string TO_KEY = "to";
        private const string TIME_SERIES_ID = "timeSeriesId";
        private const string TIME_SERIES_INTERVAL = "interval";
        private const string TIME_SERIES_AGGREGATION = "PT2M"; //this stand for 2 minutes.
        private const string TIME_SERIES_INLINE_VARIABLES = "inlineVariables";
        private const string TIME_SERIES_PROJECTED_VARIABLES = "projectedVariables";
        private const string PROPERTY_KIND = "kind";
        private const string PROPERTY_KIND_VALUE = "numeric";
        private const string TIME_SERIES_EXPRESSION = "tsx";
        private const string TIME_SERIES_EXPRESSION_FILTER = "filter";
        private const string TIME_SERIES_EXPRESSION_VALUE = "value";
        private const string TIME_SERIES_EXPRESSION_AGGREGATION = "aggregation";

        // GET api/values
        [HttpGet]
        public DeviceTelemetryModel Get()
        {
            string DeviceID = "TSITestDevice";
            string[] arr = { "temperatureC", "humidity" };
            DeviceTelemetryModel result = GetAsync(DeviceID, arr, DateTime.UtcNow.AddHours(-1), DateTime.UtcNow).Result;
            return result;
        }
        public async Task<DeviceTelemetryModel> GetAsync(string deviceid, string[] properties, DateTime fromAvailabilityTimestamp, DateTime toAvailabilityTimestamp)
        {
            //Get the access token
            string accessToken = await AcquireAccessTokenAsync();

            //Note this Azure understands time only in UTC
            DateTime from = DateTime.SpecifyKind(fromAvailabilityTimestamp, DateTimeKind.Utc);
            DateTime to = DateTime.SpecifyKind(toAvailabilityTimestamp, DateTimeKind.Utc);

            var vcontentInputPayload = PrepareInput(deviceid, properties, from, to);

            // Use HTTP request.          
            HttpWebRequest request = CreateHttpsWebRequest(environmentFqdn, "POST", "timeseries/query", accessToken, new[] { "timeout=PT20S" });

            await WriteRequestStreamAsync(request, vcontentInputPayload);

            return JsonConvert.DeserializeObject<DeviceTelemetryModel>(await GetResponseAsync(request));
        }

        private async Task<string> AcquireAccessTokenAsync()
        {
            if (ApplicationClientId == "#DUMMY#" || ApplicationClientSecret == "#DUMMY#" || Tenant.StartsWith("#DUMMY#"))
            {
                throw new Exception(
                    $"Use the link {"https://docs.microsoft.com/azure/time-series-insights/time-series-insights-authentication-and-authorization"} to update the values of 'ApplicationClientId', 'ApplicationClientSecret' and 'Tenant'.");
            }

            var authenticationContext = new AuthenticationContext(
                $"https://login.windows.net/{Tenant}",
                TokenCache.DefaultShared);

            AuthenticationResult token = await authenticationContext.AcquireTokenAsync(
                resource: "https://api.timeseries.azure.com/",
                clientCredential: new ClientCredential(
                    clientId: ApplicationClientId,
                    clientSecret: ApplicationClientSecret));

            return token.AccessToken;
        }

        private static HttpWebRequest CreateHttpsWebRequest(string host, string method, string path, string accessToken, string[] queryArgs = null)
        {
             string query = "api-version=2018-11-01-preview";
            if (queryArgs != null && queryArgs.Any())
            {
                query += "&" + String.Join("&", queryArgs);
            }

            Uri uri = new UriBuilder("https", host)
            {
                Path = path,
                Query = query
            }.Uri;
            HttpWebRequest request = WebRequest.CreateHttp(uri);
            request.Method = method;
            request.Headers.Add("x-ms-client-application-name", "AzureTSI_WebAPIExample");
            request.Headers.Add("Authorization", "Bearer " + accessToken);
            return request;
        }

        private async Task WriteRequestStreamAsync(HttpWebRequest request, JObject inputPayload)
        {
            using (var stream = await request.GetRequestStreamAsync())
            using (var streamWriter = new StreamWriter(stream))
            {
                await streamWriter.WriteAsync(inputPayload.ToString());
                await streamWriter.FlushAsync();
                streamWriter.Close();
            }
        }
        
        private async Task<string> GetResponseAsync(HttpWebRequest request)
        {
            string result = string.Empty;
            try
            {
                using (WebResponse webResponse = await request.GetResponseAsync())
                using (var sr = new StreamReader(webResponse.GetResponseStream()))
                {
                    result = await sr.ReadToEndAsync();
                }
            }
            catch (WebException e1)
            {
                var response = ((HttpWebResponse)e1.Response);
                var someheader = response.Headers["X-API-ERROR"];
                // check header
                var content = response.GetResponseStream();
                // check the content if needed 

                if (e1.Status == WebExceptionStatus.ProtocolError)
                {
                    // protocol errors find the statuscode in the Response

                    int code = (int)((HttpWebResponse)e1.Response).StatusCode;

                }
            }
            return result;
        }
         

        private JObject PrepareInput(
           string deviceid, string[] properties, DateTime from, DateTime to)
        {

            var result = new JObject();
            JObject inlineVariable = new JObject();
            JProperty propholder = null;
            JArray projectedVariablesArray = new JArray();

            if (properties != null && properties.Length > 0)
            {
                foreach (var prop in properties)
                {
                   propholder = new JProperty(prop, new JObject(
                                                     new JProperty(PROPERTY_KIND, PROPERTY_KIND_VALUE),
                                                           new JProperty(TIME_SERIES_EXPRESSION_VALUE,
                                                              new JObject(new JProperty(TIME_SERIES_EXPRESSION, "$event.[" + prop + "]"))),
                                                             new JProperty(TIME_SERIES_EXPRESSION_FILTER, null),
                                                              new JProperty(TIME_SERIES_EXPRESSION_AGGREGATION,
                                                                  new JObject(new JProperty(TIME_SERIES_EXPRESSION, "avg($value)")))
                                                   ));

                    projectedVariablesArray.Add(prop);
                    inlineVariable.AddFirst(propholder);

                }
            }

            result.Add(AGGREGATE_SERIES, new JObject(
                 new JProperty(SEARCH_SPAN_KEY, new JObject(
                        new JProperty(FROM_KEY, from.ToString(TSI_DATE_FORMAT)),
                             new JProperty(TO_KEY, to.ToString(TSI_DATE_FORMAT)))),
                             new JProperty(TIME_SERIES_ID, new JArray(deviceid)),   //incase you have multiple devices please pass it as array
                             new JProperty(TIME_SERIES_INTERVAL, TIME_SERIES_AGGREGATION), //give your time aggregation here
                             new JProperty(TIME_SERIES_INLINE_VARIABLES, inlineVariable),
                            new JProperty(TIME_SERIES_PROJECTED_VARIABLES, projectedVariablesArray)
                        ));

            return result;
        }
    }
}
