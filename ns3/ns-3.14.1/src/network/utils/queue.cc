/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "queue.h"

NS_LOG_COMPONENT_DEFINE ("Queue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Queue);

TypeId 
Queue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Queue")
    .SetParent<Object> ()
    .AddTraceSource ("Enqueue", "Enqueue a packet in the queue.",
                     MakeTraceSourceAccessor (&Queue::m_traceEnqueue))
    .AddTraceSource ("Dequeue", "Dequeue a packet from the queue.",
                     MakeTraceSourceAccessor (&Queue::m_traceDequeue))
    .AddTraceSource ("Drop", "Drop a packet stored in the queue.",
                     MakeTraceSourceAccessor (&Queue::m_traceDrop))
  ;
  return tid;
}

Queue::Queue() : 
  m_nBytes (0),
  m_nTotalReceivedBytes (0),
  m_nPackets (0),
  m_nTotalReceivedPackets (0),
  m_nTotalDroppedBytes (0),
  m_nTotalDroppedPackets (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Queue::~Queue()
{
  NS_LOG_FUNCTION_NOARGS ();
}


bool 
Queue::Enqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  //
  // If DoEnqueue fails, Queue::Drop is called by the subclass
  //
  if (m_nTotalReceivedPackets==0)
  {
    firstPacketTime=Simulator::Now();
    filteredAverageDelay[0]=0;
//    for (int i=0; i<3; i++)
//    {
//      partialDelay=(double *) malloc(sizeof(double));
//      partialPackets=malloc(sizeof(int));
//    }
//    partialDelay = { };
//    cntPackets = { };
    //double timeInterval = 100000000;
    //Time targetInterval;
    //targetInterval = ns3::Time::FromDouble(firstPacketTime.ToDouble(ns3::Time::Unit(0))+timeInterval, ns3::Time::Unit(0));
std::cout << firstPacketTime << " " << firstPacketTime.ToDouble(ns3::Time::Unit(5))/100000000 << " " << int(firstPacketTime.ToDouble(ns3::Time::Unit(0))*10) << " " << firstPacketTime.ToDouble(ns3::Time::Unit(0)) << " " << firstPacketTime.GetNanoSeconds()/100000000 << std::endl;
  }

  bool retval = DoEnqueue (p);
  if (retval)
    {
      NS_LOG_LOGIC ("m_traceEnqueue (p)");
      m_traceEnqueue (p);

      m_pktDelay.insert (std::pair <Ptr<Packet>, Time> (p, Simulator::Now() ));

      uint32_t size = p->GetSize ();
      m_nBytes += size;
      m_nTotalReceivedBytes += size;

      m_nPackets++;
      m_nTotalReceivedPackets++;
    }
  return retval;
}

Ptr<Packet>
Queue::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet = DoDequeue ();
  std::map< Ptr<Packet>, Time >::iterator it;
  it = m_pktDelay.find (packet);
  Time now = Simulator::Now();
  Time delay;
  delay = now - (*it).second;

  int index = int((Simulator::Now()-firstPacketTime).GetNanoSeconds()/100000000)+1;
//  ptrpartialDelay = (double *) realloc(partialDelay, (index+1)*sizeof(double));
  partialDelay[index] = partialDelay[index] + delay.GetNanoSeconds();
//  partialDelay[int((Simulator::Now()-firstPacketTime).GetNanoSeconds()/100000000)] = partialDelay[(Simulator::Now()-firstPacketTime).GetNanoSeconds()/100000000] + double(delay.GetNanoSeconds());
  partialPackets[index]++;

  m_pktDelay.erase (it);
  if (index>1)
    filteredAverageDelay[index] = 0.9*filteredAverageDelay[index-1] + 0.1*partialDelay[index]/partialPackets[index];
  else
    filteredAverageDelay[index] =partialDelay[index]/partialPackets[index];
//  filteredAverageDelay = 0.9*filteredAverageDelay+0.1*double(delay.GetNanoSeconds());

  totalDelay += delay.GetNanoSeconds();
  double delaySquared = (delay.GetSeconds())*(delay.GetSeconds());
  ss[index] = ss[index] + delaySquared;
  delayDeviation[index] = sqrt((ss[index]-(partialDelay[index]/1000000000)*(partialDelay[index]/1000000000)/partialPackets[index])/(partialPackets[index]-1));

//  cntDelay ++;

//  =====
//inside the event... 
//opt 1: (100-ms memory)
// filteredAverageDelay = totalDelay/cntDelay 

//opt 2: (1-sec memory) 
//filteredAverageDelay = 0.1*totalDelay/cntDelay + 0.9*filteredAverageDelay; 
  globalAverageDelay = totalDelay/m_nTotalReceivedPackets;
//   =====



std::cout << "SimulationTime[ns]: " << Simulator::Now() << " FilteredAverageDelay[microsec]: " << filteredAverageDelay[index]/1000 << " InstantaneousDelay[ns]: " << delay << " GlobalAverageDelay[microsec]: " << globalAverageDelay/1000 << " partialDelay: " << partialDelay[int((Simulator::Now()-firstPacketTime).GetNanoSeconds()/100000000)] << " partialPackets[" << index << "]: " << partialPackets[index] << std::endl; 
//  std::cout << "Delay: " << delay << " totalDelay: " << totalDelay << " AvgeDelay: " << avgeDelay << " Packets:" << m_nTotalReceivedPackets << std::endl;

  if (packet != 0)
    {
      NS_ASSERT (m_nBytes >= packet->GetSize ());
      NS_ASSERT (m_nPackets > 0);

      m_nBytes -= packet->GetSize ();
      m_nPackets--;

      NS_LOG_LOGIC ("m_traceDequeue (packet)");
      m_traceDequeue (packet);
    }
  return packet;
}

void
Queue::DequeueAll (void)
{
  NS_LOG_FUNCTION (this);
  while (!IsEmpty ())
    {
      Dequeue ();
    }
}

Ptr<const Packet>
Queue::Peek (void) const
{
  NS_LOG_FUNCTION (this);
  return DoPeek ();
}


uint32_t 
Queue::GetNPackets (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("returns " << m_nPackets);
  return m_nPackets;
}

uint32_t
Queue::GetNBytes (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC (" returns " << m_nBytes);
  return m_nBytes;
}

bool
Queue::IsEmpty (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("returns " << (m_nPackets == 0));
  return m_nPackets == 0;
}

uint32_t
Queue::GetTotalReceivedBytes (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("returns " << m_nTotalReceivedBytes);
  return m_nTotalReceivedBytes;
}

uint32_t
Queue::GetTotalReceivedPackets (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("returns " << m_nTotalReceivedPackets);
  return m_nTotalReceivedPackets;
}

uint32_t
Queue:: GetTotalDroppedBytes (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("returns " << m_nTotalDroppedBytes);
  return m_nTotalDroppedBytes;
}

uint32_t
Queue::GetTotalDroppedPackets (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("returns " << m_nTotalDroppedPackets);
  return m_nTotalDroppedPackets;
}

void 
Queue::ResetStatistics (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nTotalReceivedBytes = 0;
  m_nTotalReceivedPackets = 0;
  m_nTotalDroppedBytes = 0;
  m_nTotalDroppedPackets = 0;
}

void
Queue::Drop (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  m_nTotalDroppedPackets++;
  m_nTotalDroppedBytes += p->GetSize ();

  NS_LOG_LOGIC ("m_traceDrop (p)");
  m_traceDrop (p);
}

} // namespace ns3
