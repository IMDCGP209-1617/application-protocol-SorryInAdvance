using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;
using System.Text.RegularExpressions;

namespace CSharpClient
{
    public partial class ChatClient : Form
    {
        public class StateObject
        {
            public Socket WorkSocket = null;
            public const int BufferSize = 256;
            // Receive buffer.  
            public byte[] Buffer = new byte[BufferSize];
            // Received data string.  
            public StringBuilder Sb = new StringBuilder();
        }

        private static string RESPONSE = string.Empty;
        private static readonly ManualResetEvent receiveDone = new ManualResetEvent(false);

        private IPAddress serverAddress;
        private int serverPort;

        private IPEndPoint endPoint;

        private Socket client;

        private StateObject state;

        string room;

        public ChatClient()
        {
            InitializeComponent();

        }

        private void List_RightClick(object sender, MouseEventArgs e)
        {

            if (e.Button == MouseButtons.Right)
            {
                int index = this.listBox1.IndexFromPoint(e.Location);
                if (index != ListBox.NoMatches)
                {
                    listBox1.SelectedIndex = index;
                    contextMenuStrip1.Show(listBox1, e.Location);
                }
            }
            if (e.Button == MouseButtons.Left)
            {
                int index = this.listBox2.IndexFromPoint(e.Location);
                if (index != ListBox.NoMatches)
                {
                    listBox2.SelectedIndex = index;
                    contextMenuStrip2.Show(listBox2, e.Location);
                }
            }

        }
        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var connectForm = new ConnectDialog();

            if (connectForm.ShowDialog() != DialogResult.OK)
                return;

            var validAddress = connectForm.GetAddress(out serverAddress, out serverPort);

            if (!validAddress)
                return;

            Disconnect();

            endPoint = new IPEndPoint(serverAddress, serverPort);
            client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            client.Connect(endPoint);

            if (!client.Connected) return;

            state = new StateObject { WorkSocket = client };
            disconnectToolStripMenuItem.Enabled = true;

            // Begin receiving the data from the remote device.  
            client.BeginReceive(state.Buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReceiveCallback), state);
        }

        public delegate void UpdateMessageBox();

        public void UpdateMessages()
        {
            if (messageText.InvokeRequired)
            {
                string[] result;
                string[] sHold = null;
              

                result = state.Sb.ToString().Split(' ');//split input by spaces
                string temp = "";
                
                
                if (result[0] == "Open" || result[0] == "Matching")//if reciving a room related command adjust the output to be displayed in a lisbox
                {
                    
                    for (int i = 2; i < result.Length - 1; i++)
                    {
                        temp = temp + result[i];
                    }
                    sHold = temp.Split(',');
                    List<string> Rooms = new List<string>();
                    for (int i = 0; i < sHold.Length-1; i++)
                    {
  
                            Rooms.Add(sHold[i]);
                        

                    }
                    
                    messageText.Invoke((MethodInvoker)delegate { listBox1.DataSource = Rooms; });


                }else if(result[0] == "People")//if reciving a user related command adjust the output to be displayed in a lisbox
                {
                    for (int i = 4; i < result.Length - 1; i++)
                    {
                        temp = temp + result[i];
                    }
                    sHold = temp.Split(',');
                    List<string> users = new List<string>();
                    for (int i = 0; i < sHold.Length - 1; i++)
                    {
                        
                            users.Add(sHold[i]);
                        
                    }
                    messageText.Invoke((MethodInvoker)delegate { listBox2.DataSource = users; });
                }
                else if (result[0] == "<Page")//if reciving /command display it in a seperate text box 
                {
                    messageText.Invoke((MethodInvoker)delegate { textBox3.AppendText(state.Sb.ToString() + "\n"); });
                }
                else
                {
                    messageText.Invoke((MethodInvoker)delegate { messageText.AppendText(state.Sb.ToString() + "\n"); });
                }
                

                state.Sb.Clear();
            }
        }

        private void ReceiveCallback(IAsyncResult ar)
        {
            var stateObject = (StateObject) ar.AsyncState;
            var workSocket = stateObject.WorkSocket;

            // Read data from the remote device.  
            var bytesRead = client.EndReceive(ar);

            if (bytesRead > 0)
            {
                // There might be more data, so store the data received so far.  
                stateObject.Sb.Append(Encoding.ASCII.GetString(stateObject.Buffer, 0, bytesRead));

                UpdateMessages();
            }

            // Get the rest of the data.  
            client.BeginReceive(stateObject.Buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReceiveCallback), stateObject);
        }

        private void Disconnect()
        {
            if (client == null || !client.Connected)
                return;

            client.Disconnect(true);
            disconnectToolStripMenuItem.Enabled = false;
            client.Shutdown(SocketShutdown.Both);
        }

        private void disconnectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Disconnect();
        }

        private void Send(string msg)
        {
            if (!client.Connected)
                return;

            var msgBytes = Encoding.ASCII.GetBytes(msg);

            client.Send(msgBytes);
        }

        private void sendBtn_Click(object sender, EventArgs e)
        {
            var msg = messageBox.Text;

            string[] tempStr = messageBox.Text.Split(' ');
            if(tempStr[0] == "/whosin")//formats a /whosin command so it can be sent
            {
                room = tempStr[1];
            }

            Send(msg);
        }

        private void joinToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //formats a /join command so it can be sent
            var msg = "/join " + listBox1.Text;

            Send(msg);
        }

        private void leaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //formats a /leave command so it can be sent
            var msg = "/leave " + listBox1.Text;

            Send(msg);
        }

        private void kickToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //formats a /kick command so it can be sent
            var msg = "/kick " + listBox2.Text;

            Send(msg);
        }

        private void banToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //formats a /ban command so it can be sent
            var msg = "/ban " + listBox2.Text +" "+ room;

            Send(msg);
        }
    }
}
