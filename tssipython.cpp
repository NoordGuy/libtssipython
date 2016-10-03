/*++
*    libtssipython - Python wrapper for libtssi
           parse MPEG-2 TS and DVB Service Information in Python
*
*    Copyright (C) 2009, 2016 Martin Hoernig (goforcode.com)
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
--*/

#ifndef __TSSIPYTHON_H_INCLUDED__
#define __TSSIPYTHON_H_INCLUDED__

#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/stl_iterator.hpp> 
#include <vector>

using namespace boost::python;

#include "tssi.h"

// python buffer wrapper
static TS_BOOL ParserProcessPython(tssi::Parser& self, object py_buffer) {
	// from http://stackoverflow.com/questions/32634765/how-do-i-pass-a-pre-populated-unsigned-char-buffer-to-a-c-method-using-boos

	// `str` objects do not implement the iterator protcol (__iter__),
	// but do implement the sequence protocol (__getitem__).  Use the
	// `iter()` builtin to create an iterator for the buffer.
	// >>> __builtins__.iter(py_buffer)
	object locals(borrowed(PyEval_GetLocals()));
	object py_iter = locals["__builtins__"].attr("iter");
	stl_input_iterator<unsigned char> begin(py_iter(py_buffer)), end;

	// Copy the py_buffer into a local buffer with known continguous memory.
	std::vector<unsigned char> buffer(begin, end);

	// Cast and delegate to the printBuffer member function.
	return self.Process(reinterpret_cast<unsigned char*>(&buffer[0]), static_cast<unsigned> ( buffer.size() ));

}

// wrapping overloaded functions
const tssi::EbuPage& (tssi::Packet_Ebu::*GetEbuPage1) (unsigned) const = &tssi::Packet_Ebu::GetEbuPage;
const tssi::EbuPage& (tssi::Packet_Ebu::*GetEbuPage2) (TS_BYTE, TS_BYTE, TS_WORD) const = &tssi::Packet_Ebu::GetEbuPage;

unsigned (tssi::DescriptorList::*GetDescriptorListLength1) () const = &tssi::DescriptorList::GetDescriptorListLength;
unsigned (tssi::DescriptorList::*GetDescriptorListLength2) (TS_BYTE) const = &tssi::DescriptorList::GetDescriptorListLength;

tssi::Descriptor* (tssi::DescriptorList::*GetDescriptorByTag1) (TS_BYTE) const = &tssi::DescriptorList::GetDescriptorByTag;
tssi::Descriptor* (tssi::DescriptorList::*GetDescriptorByTag2) (TS_BYTE, unsigned) const = &tssi::DescriptorList::GetDescriptorByTag;

// callback wrappers
TS_VOID PythonCallback(TS_PVOID data) {
	PyObject *callback = reinterpret_cast<PyObject *> (data);

	PyEval_InitThreads();
	PyGILState_STATE state = PyGILState_Ensure();

	call<void>(callback);

	PyGILState_Release(state);
}

TS_VOID SetEbuCallback(tssi::Packet_Ebu& self, object py_callback) {	self.SetNewPageCallback(&PythonCallback, &py_callback); }
TS_VOID SetPcrCallback(tssi::Packet_Pcr& self, object py_callback) {	self.SetPcrCallback(&PythonCallback, &py_callback); }
template<class T> TS_VOID SetTableCallback(T& self, object py_callback) { self.SetProcessCallback(&PythonCallback, &py_callback); }

BOOST_PYTHON_MODULE(libtssipython)
{
	class_<tssi::Descriptor, boost::noncopyable>("Descriptor")
		.def("Reset", &tssi::Descriptor::Reset)
		.def("GetDescriptorTag", &tssi::Descriptor::GetDescriptorTag)
		.def("GetDescriptorLength", &tssi::Descriptor::GetDescriptorLength)
	;

	class_<tssi::Descriptor_Application, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Application")
		.def("Reset", &tssi::Descriptor_Application::Reset)	
		.def("GetVersionListLength", &tssi::Descriptor_Application::GetVersionListLength)	
		.def("GetApplicationProfile", &tssi::Descriptor_Application::GetApplicationProfile)	
		.def("GetVersionMajor", &tssi::Descriptor_Application::GetVersionMajor)	
		.def("GetVersionMinor", &tssi::Descriptor_Application::GetVersionMinor)	
		.def("GetVersionMicro", &tssi::Descriptor_Application::GetVersionMicro)	
		.def("GetServiceBound", &tssi::Descriptor_Application::GetServiceBound)	
		.def("GetVisibility", &tssi::Descriptor_Application::GetVisibility)	
		.def("GetApplicationPriority", &tssi::Descriptor_Application::GetApplicationPriority)	
	;

	class_<tssi::Descriptor_ApplicationName, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_ApplicationName")
		.def("Reset", &tssi::Descriptor_ApplicationName::Reset)	
		.def("GetApplicationNameListLength", &tssi::Descriptor_ApplicationName::GetApplicationNameListLength)	
		.def("GetApplicationName", &tssi::Descriptor_ApplicationName::GetApplicationName, return_value_policy<copy_const_reference>())	
		.def("GetApplicationNameLanguage", &tssi::Descriptor_ApplicationName::GetApplicationNameLanguage)	
	;

	class_<tssi::Descriptor_ApplicationSignalling, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_ApplicationSignalling")
		.def("Reset", &tssi::Descriptor_ApplicationSignalling::Reset)	
		.def("GetListLength", &tssi::Descriptor_ApplicationSignalling::GetListLength)	
		.def("GetApplicationType", &tssi::Descriptor_ApplicationSignalling::GetApplicationType)	
		.def("GetAitVersion", &tssi::Descriptor_ApplicationSignalling::GetAitVersion)	
	;

	class_<tssi::Descriptor_Component, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Component")
		.def("Reset", &tssi::Descriptor_Component::Reset)	
		.def("GetText", &tssi::Descriptor_Component::GetText, return_value_policy<copy_const_reference>())	
		.def("GetStreamContent", &tssi::Descriptor_Component::GetStreamContent)	
		.def("GetComponentType", &tssi::Descriptor_Component::GetComponentType)	
		.def("GetComponentTag", &tssi::Descriptor_Component::GetComponentTag)	
		.def("GetLanguageCode", &tssi::Descriptor_Component::GetLanguageCode)	
	;

	class_<tssi::Descriptor_Compression, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Compression")
		.def("Reset", &tssi::Descriptor_Compression::Reset)	
		.def("GetCompressionSystem", &tssi::Descriptor_Compression::GetCompressionSystem)	
		.def("GetOriginalSize", &tssi::Descriptor_Compression::GetOriginalSize)	
	;

	class_<tssi::Descriptor_Content, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Content")
		.def("Reset", &tssi::Descriptor_Content::Reset)	
		.def("GetNibbleListLength", &tssi::Descriptor_Content::GetNibbleListLength)	
		.def("GetContentNibbleLevel1", &tssi::Descriptor_Content::GetContentNibbleLevel1)	
		.def("GetContentNibbleLevel2", &tssi::Descriptor_Content::GetContentNibbleLevel2)	
	;

	class_<tssi::Descriptor_DvbJApplication, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_DvbJApplication")
		.def("Reset", &tssi::Descriptor_DvbJApplication::Reset)	
		.def("GetParameterListLength", &tssi::Descriptor_DvbJApplication::GetParameterListLength)	
		.def("GetParameter", &tssi::Descriptor_DvbJApplication::GetParameter, return_value_policy<copy_const_reference>())	
	;

	class_<tssi::Descriptor_DvbJApplicationLocation, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_DvbJApplicationLocation")
		.def("Reset", &tssi::Descriptor_DvbJApplicationLocation::Reset)	
		.def("GetBaseDirectory", &tssi::Descriptor_DvbJApplicationLocation::GetBaseDirectory, return_value_policy<copy_const_reference>())	
		.def("GetClasspathExtension", &tssi::Descriptor_DvbJApplicationLocation::GetClasspathExtension, return_value_policy<copy_const_reference>())	
		.def("GetInitialClass", &tssi::Descriptor_DvbJApplicationLocation::GetInitialClass, return_value_policy<copy_const_reference>())	
	;

	class_<tssi::Descriptor_ExtendedEvent, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_ExtendedEvent")
		.def("Reset", &tssi::Descriptor_ExtendedEvent::Reset)	
		.def("GetEventLanguage", &tssi::Descriptor_ExtendedEvent::GetEventLanguage)	
		.def("GetEventDescripton", &tssi::Descriptor_ExtendedEvent::GetEventDescripton, return_value_policy<copy_const_reference>())	
		.def("GetDescriptorNumber", &tssi::Descriptor_ExtendedEvent::GetDescriptorNumber)	
		.def("GetLastDescriptorNumber", &tssi::Descriptor_ExtendedEvent::GetLastDescriptorNumber)	
	;

	class_<tssi::Descriptor_FrameRate, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_FrameRate")
		.def("Reset", &tssi::Descriptor_FrameRate::Reset)	
		.def("GetMultipleFrameRateFlag", &tssi::Descriptor_FrameRate::GetMultipleFrameRateFlag)	
		.def("GetFrameRateCode", &tssi::Descriptor_FrameRate::GetFrameRateCode)	
	;

	class_<tssi::Descriptor_Iso639, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Iso639")
		.def("Reset", &tssi::Descriptor_Iso639::Reset)	
		.def("GetLanguageListLength", &tssi::Descriptor_Iso639::GetLanguageListLength)	
		.def("GetLanguageCode", &tssi::Descriptor_Iso639::GetLanguageCode)	
		.def("GetLanguageType", &tssi::Descriptor_Iso639::GetLanguageType)	
	;
	
	class_<tssi::Descriptor_NetworkName, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_NetworkName")
		.def("Reset", &tssi::Descriptor_NetworkName::Reset)	
		.def("GetNetworkName", &tssi::Descriptor_NetworkName::GetNetworkName, return_value_policy<copy_const_reference>())	
	;

	class_<tssi::Descriptor_Pdc, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Pdc")
		.def("Reset", &tssi::Descriptor_Pdc::Reset)	
		.def("GetPil", &tssi::Descriptor_Pdc::GetPil)	
	;
	
	enum_<tssi::SatelliteDeliveryPolarization::Enum>("SatelliteDeliveryPolarization")
		.value("LINEAR_HORIZONTAL", tssi::SatelliteDeliveryPolarization::LINEAR_HORIZONTAL)
		.value("LINEAR_VERTICAL", tssi::SatelliteDeliveryPolarization::LINEAR_VERTICAL)
		.value("CIRCULAR_HORIZONTAL", tssi::SatelliteDeliveryPolarization::CIRCULAR_HORIZONTAL)
		.value("CIRCULAR_VERTICAL", tssi::SatelliteDeliveryPolarization::CIRCULAR_VERTICAL)
	;

	enum_<tssi::SatelliteDeliveryModulation::Enum>("SatelliteDeliveryModulation")
		.value("NOT_DEFINED", tssi::SatelliteDeliveryModulation::NOT_DEFINED)
		.value("QPSK", tssi::SatelliteDeliveryModulation::QPSK)
		.value("M8PSK", tssi::SatelliteDeliveryModulation::M8PSK)
		.value("M16_QAM", tssi::SatelliteDeliveryModulation::M16_QAM)
	;

	enum_<tssi::SatelliteDeliveryFec::Enum>("SatelliteDeliveryFec")
		.value("NOT_DEFINED", tssi::SatelliteDeliveryFec::NOT_DEFINED)
		.value("F1_2", tssi::SatelliteDeliveryFec::F1_2)
		.value("F2_3", tssi::SatelliteDeliveryFec::F2_3)
		.value("F3_4", tssi::SatelliteDeliveryFec::F3_4)
		.value("F5_6", tssi::SatelliteDeliveryFec::F5_6)
		.value("F7_8", tssi::SatelliteDeliveryFec::F7_8)
		.value("F8_9", tssi::SatelliteDeliveryFec::F8_9)
		.value("NO", tssi::SatelliteDeliveryFec::NO)
	;
	
	class_<tssi::Descriptor_SatelliteDelivery, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_SatelliteDelivery")
		.def("Reset", &tssi::Descriptor_SatelliteDelivery::Reset)	
		.def("GetFrequency", &tssi::Descriptor_SatelliteDelivery::GetFrequency)	
		.def("GetOrbitalPosition", &tssi::Descriptor_SatelliteDelivery::GetOrbitalPosition)	
		.def("GetWestEastFlag", &tssi::Descriptor_SatelliteDelivery::GetWestEastFlag)	
		.def("GetSymbolRate", &tssi::Descriptor_SatelliteDelivery::GetSymbolRate)	
		.def("GetPolarization", &tssi::Descriptor_SatelliteDelivery::GetPolarization)	
		.def("GetModulation", &tssi::Descriptor_SatelliteDelivery::GetModulation)	
		.def("GetFecInner", &tssi::Descriptor_SatelliteDelivery::GetFecInner)	
	;
	
	class_<tssi::Descriptor_Service, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Service")
		.def("Reset", &tssi::Descriptor_Service::Reset)	
		.def("GetServiceType", &tssi::Descriptor_Service::GetServiceType)	
		.def("GetProviderName", &tssi::Descriptor_Service::GetProviderName,return_value_policy<copy_const_reference>())	
		.def("GetServiceName", &tssi::Descriptor_Service::GetServiceName, return_value_policy<copy_const_reference>())	
	;

	class_<tssi::Descriptor_ShortEvent, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_ShortEvent")
		.def("Reset", &tssi::Descriptor_ShortEvent::Reset)	
		.def("GetEventName", &tssi::Descriptor_ShortEvent::GetEventName, return_value_policy<copy_const_reference>())	
		.def("GetEventText", &tssi::Descriptor_ShortEvent::GetEventText,return_value_policy<copy_const_reference>())	
	;

	class_<tssi::Descriptor_Teletext, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_Teletext")
		.def("Reset", &tssi::Descriptor_Teletext::Reset)	
		.def("GetTeletextListLength", &tssi::Descriptor_Teletext::GetTeletextListLength)	
		.def("GetTeletextLanguage", &tssi::Descriptor_Teletext::GetTeletextLanguage)	
		.def("GetTeletextType", &tssi::Descriptor_Teletext::GetTeletextType)	
		.def("GetTeletextMagazineNumber", &tssi::Descriptor_Teletext::GetTeletextMagazineNumber)	
		.def("GetTeletextPageNumber", &tssi::Descriptor_Teletext::GetTeletextPageNumber)	
	;
	
	class_<tssi::Descriptor_TransportProtocol, bases<tssi::Descriptor>, boost::noncopyable>("Descriptor_TransportProtocol")
		.def("Reset", &tssi::Descriptor_TransportProtocol::Reset)	
		.def("GetProtocolId", &tssi::Descriptor_TransportProtocol::GetProtocolId)	
	;

	class_<tssi::DescriptorList, boost::noncopyable>("DescriptorList")
		.def("Reset", &tssi::DescriptorList::Reset)	
		.def("GetDescriptorListLength", GetDescriptorListLength1)	
		.def("GetDescriptorLength", &tssi::DescriptorList::GetDescriptorLength)	
		.def("GetDescriptorTag", &tssi::DescriptorList::GetDescriptorTag)	
		.def("GetDescriptorListLength", GetDescriptorListLength2)	
		.def("GetDescriptor", &tssi::DescriptorList::GetDescriptor, return_internal_reference<>())	
		.def("GetDescriptorByTag", GetDescriptorByTag1, return_internal_reference<>())	
		.def("GetDescriptorByTag", GetDescriptorByTag2, return_internal_reference<>())	
	;

	class_<tssi::EbuLine, boost::noncopyable>("EbuLine")
		.def_readonly("packet", &tssi::EbuLine::packet)
		.def("GetLineData", &tssi::EbuLine::GetLineData)	
	;

	class_<tssi::EbuPage, boost::noncopyable>("EbuPage")
		.def_readonly("magazine", &tssi::EbuPage::magazine)
		.def_readonly("page_number", &tssi::EbuPage::page_number)
		.def_readonly("sub_page_number", &tssi::EbuPage::sub_page_number)
		.def_readonly("erase_page_flag", &tssi::EbuPage::erase_page_flag)
		.def_readonly("newsflash_flag", &tssi::EbuPage::newsflash_flag)
		.def_readonly("subtitle_flag", &tssi::EbuPage::subtitle_flag)
		.def_readonly("suppress_header_flag", &tssi::EbuPage::suppress_header_flag)
		.def_readonly("update_indicator_flag", &tssi::EbuPage::update_indicator_flag)
		.def_readonly("interrupted_sequence_flag", &tssi::EbuPage::interrupted_sequence_flag)
		.def_readonly("inhibit_display_flag", &tssi::EbuPage::inhibit_display_flag)
		.def_readonly("magazine_serial_flag", &tssi::EbuPage::magazine_serial_flag)
		.def_readonly("language_code", &tssi::EbuPage::language_code)
		.def("GetHeaderData", &tssi::EbuPage::GetHeaderData)	
		.def("GetPageLineListLength", &tssi::EbuPage::GetPageLineListLength)	
		.def("GetPageLine", &tssi::EbuPage::GetPageLine, return_internal_reference<>())	
	;

	class_<tssi::Packet_Ebu, boost::noncopyable>("Packet_Ebu")
		.def("Reset", &tssi::Packet_Ebu::Reset)	
		.def("GetEbuPageListLength", &tssi::Packet_Ebu::GetEbuPageListLength)	
		.def("GetEbuPage", GetEbuPage1, return_internal_reference<>())
		.def("GetEbuPage", GetEbuPage2, return_internal_reference<>())
		.def("GetCurrentTimeHeader", &tssi::Packet_Ebu::GetCurrentTimeHeader)
		.def("SetNewPageCallback", &SetEbuCallback)
	;

	class_<tssi::Packet_Pcr, boost::noncopyable>("Packet_Pcr")
		.def("Reset", &tssi::Packet_Pcr::Reset)	
		.def("GetPcr", &tssi::Packet_Pcr::GetPcr)	
		.def("SetPcrCallback", &SetPcrCallback)	
	;

	class_<tssi::Table_Ait, boost::noncopyable>("Table_Ait")
		.def("Reset", &tssi::Table_Ait::Reset)	
		.def("GetTestApplicationFlag", &tssi::Table_Ait::GetTestApplicationFlag)	
		.def("GetApplicationType", &tssi::Table_Ait::GetApplicationType)	
		.def("GetCommonDescriptorList", &tssi::Table_Ait::GetCommonDescriptorList, return_internal_reference<>())	
		.def("GetApplicationListLength", &tssi::Table_Ait::GetApplicationListLength)	
		.def("GetOrganisationId", &tssi::Table_Ait::GetOrganisationId)	
		.def("GetApplicationId", &tssi::Table_Ait::GetApplicationId)	
		.def("GetApplicationControlCode", &tssi::Table_Ait::GetApplicationControlCode)	
		.def("GetApplicationDescriptorList", &tssi::Table_Ait::GetApplicationDescriptorList, return_internal_reference<>())	
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Ait>, return_internal_reference<>())	
	;

	class_<tssi::EitEvent, boost::noncopyable>("EitEvent")
		.def_readonly("transport_stream_id", &tssi::EitEvent::transport_stream_id)
		.def_readonly("original_network_id", &tssi::EitEvent::original_network_id)
		.def_readonly("service_id", &tssi::EitEvent::service_id)
		.def_readonly("event_id", &tssi::EitEvent::event_id)
		.def_readonly("start_time", &tssi::EitEvent::start_time)
		.def_readonly("duration", &tssi::EitEvent::duration)
		.def_readonly("running_status", &tssi::EitEvent::running_status)
		.def_readonly("free_ca_mode", &tssi::EitEvent::free_ca_mode)
		.def_readonly("descriptor_list", &tssi::EitEvent::descriptor_list)
	;

	class_<tssi::Table_Eit, boost::noncopyable>("Table_Eit")
		.def("Reset", &tssi::Table_Eit::Reset)	
		.def("GetEventListLength", &tssi::Table_Eit::GetEventListLength)	
		.def("GetEvent", &tssi::Table_Eit::GetEvent, return_internal_reference<>())	
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Eit>, return_internal_reference<>())	
	;

	class_<tssi::Table_Nit, boost::noncopyable>("Table_Nit")
		.def("Reset", &tssi::Table_Nit::Reset)
		.def("GetNetworkId", &tssi::Table_Nit::GetNetworkId)
		.def("GetCommonDescriptorList", &tssi::Table_Nit::GetCommonDescriptorList, return_internal_reference<>())
		.def("GetNetworkListLength", &tssi::Table_Nit::GetNetworkListLength)
		.def("GetNetworkTransportStreamId", &tssi::Table_Nit::GetNetworkTransportStreamId)
		.def("GetNetworkOriginalNetworkId", &tssi::Table_Nit::GetNetworkOriginalNetworkId)
		.def("GetNetworkDescriptorList", &tssi::Table_Nit::GetNetworkDescriptorList, return_internal_reference<>())
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Nit>, return_internal_reference<>())	
	;

	class_<tssi::Table_Pat, boost::noncopyable>("Table_Pat")
		.def("Reset", &tssi::Table_Pat::Reset)
		.def("GetTransportStreamId", &tssi::Table_Pat::GetTransportStreamId)
		.def("GetNetworkPid", &tssi::Table_Pat::GetNetworkPid)
		.def("GetProgramListLength", &tssi::Table_Pat::GetProgramListLength)
		.def("GetProgramNumber", &tssi::Table_Pat::GetProgramNumber)
		.def("GetProgramMapPid", &tssi::Table_Pat::GetProgramMapPid)
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Pat>, return_internal_reference<>())	
	;

	class_<tssi::Table_Pmt, boost::noncopyable>("Table_Pmt")
		.def("Reset", &tssi::Table_Pmt::Reset)
		.def("GetProgramListLength", &tssi::Table_Pmt::GetProgramListLength)
		.def("GetProgramNumber", &tssi::Table_Pmt::GetProgramNumber)
		.def("GetPcrPid", &tssi::Table_Pmt::GetPcrPid)
		.def("GetEsListLength", &tssi::Table_Pmt::GetEsListLength)
		.def("GetEsType", &tssi::Table_Pmt::GetEsType)
		.def("GetEsPid", &tssi::Table_Pmt::GetEsPid)
		.def("GetEsDescriptorList", &tssi::Table_Pmt::GetEsDescriptorList, return_internal_reference<>())
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Pmt>, return_internal_reference<>())	
	;

	class_<tssi::ServiceDescription, boost::noncopyable>("ServiceDescription")
		.def_readonly("transport_stream_id", &tssi::ServiceDescription::transport_stream_id)
		.def_readonly("original_network_id", &tssi::ServiceDescription::original_network_id)
		.def_readonly("service_id", &tssi::ServiceDescription::service_id)
		.def_readonly("eit_schedule_flag", &tssi::ServiceDescription::eit_schedule_flag)
		.def_readonly("eit_present_following_flag", &tssi::ServiceDescription::eit_present_following_flag)
		.def_readonly("running_status", &tssi::ServiceDescription::running_status)
		.def_readonly("free_ca_mode", &tssi::ServiceDescription::free_ca_mode)
		.def_readonly("descriptor_list", &tssi::ServiceDescription::descriptor_list)
	;

	class_<tssi::Table_Sdt, boost::noncopyable>("Table_Sdt")
		.def("Reset", &tssi::Table_Sdt::Reset)
		.def("GetServiceListLength", &tssi::Table_Sdt::GetServiceListLength)
		.def("GetServiceDescription", &tssi::Table_Sdt::GetServiceDescription, return_internal_reference<>())
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Sdt>, return_internal_reference<>())	
	;

	class_<tssi::Table_Dsmcc, boost::noncopyable>("Table_Dsmcc")
		.def("Reset", &tssi::Table_Dsmcc::Reset)
		.def("GetDownloadListLength", &tssi::Table_Dsmcc::GetDownloadListLength)
		.def("IsDownloadComplete", &tssi::Table_Dsmcc::IsDownloadComplete)
		.def("ProcessDownload", &tssi::Table_Dsmcc::ProcessDownload)
		.def("Decode", &tssi::Table_Dsmcc::Decode)
	;

	class_<TS_TIME>("TS_TIME")
		.def_readonly("year", &TS_TIME::year)
		.def_readonly("month", &TS_TIME::month)
		.def_readonly("day_of_week", &TS_TIME::day_of_week)
		.def_readonly("day", &TS_TIME::day)
		.def_readonly("minute", &TS_TIME::minute)
		.def_readonly("second", &TS_TIME::second)
		.def_readonly("milliseconds", &TS_TIME::milliseconds)
	;

	class_<tssi::Table_Tdt, boost::noncopyable>("Table_Tdt")
		.def("Reset", &tssi::Table_Tdt::Reset)
		.def("GetTime", &tssi::Table_Tdt::GetTime)
		.def("GetSnapshotTime", &tssi::Table_Tdt::GetSnapshotTime)
		.def("SetProcessCallback", &SetTableCallback<tssi::Table_Tdt>, return_internal_reference<>())	
	;

	class_<tssi::Parser, boost::noncopyable>("Parser")
		.def("Reset", &tssi::Parser::Reset)	
		.def("Process", &ParserProcessPython, (arg("self"), arg("py_buffer")))
		.def("SetPidDsmcc", &tssi::Parser::SetPidDsmcc)	
		.def("SetPidAit", &tssi::Parser::SetPidAit)	
		.def("SetPidPcr", &tssi::Parser::SetPidPcr)	
		.def("SetPidEbu", &tssi::Parser::SetPidEbu)	
		.def("ProcessingErrors", &tssi::Parser::ProcessingErrors)	
		.def("PacketsProcessed", &tssi::Parser::PacketsProcessed)	
		.def("PacketEbu", &tssi::Parser::PacketEbu, return_internal_reference<>())
		.def("PacketPcr", &tssi::Parser::PacketPcr, return_internal_reference<>())
		.def("TableAit", &tssi::Parser::TableAit, return_internal_reference<>())
		.def("TableDsmcc", &tssi::Parser::TableDsmcc, return_internal_reference<>())
		.def("TableEit", &tssi::Parser::TableEit, return_internal_reference<>())
		.def("TableNit", &tssi::Parser::TableNit, return_internal_reference<>())
		.def("TablePat", &tssi::Parser::TablePat, return_internal_reference<>())
		.def("TablePmt", &tssi::Parser::TablePmt, return_internal_reference<>())
		.def("TableSdt", &tssi::Parser::TableSdt, return_internal_reference<>())
		.def("TableTdt", &tssi::Parser::TableTdt, return_internal_reference<>())
	;

}

#endif // __TSSIPYTHON_H_INCLUDED__