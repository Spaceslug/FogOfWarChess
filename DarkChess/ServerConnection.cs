using Grpc.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static ChessCom.ChessCom;

namespace DarkChess
{
    public class ServerConnection
    {
        public string IpAdress { get; private set; }
        public int Port { get; private set; }
        private Channel _channel;
        private ChessComClient _client;

        public ChessComClient Call
        {
            get { return _client; }
        }

        public ServerConnection(string ip, int port)
        {
            IpAdress = ip;
            Port = port;
        }

        public void Connect()
        {
            _channel = new Channel(IpAdress, Port, ChannelCredentials.Insecure);
            _channel.ConnectAsync(DateTime.UtcNow + TimeSpan.FromSeconds(10)).Wait();
            _client = new ChessComClient(_channel);
        }

        public void ShutDownAsync()
        {
            _channel?.ShutdownAsync();
        }
    }
}
