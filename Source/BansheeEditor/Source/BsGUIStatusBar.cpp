//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIStatusBar.h"
#include "BsGUILayoutX.h"
#include "BsGUILayoutY.h"
#include "BsGUILabel.h"
#include "BsGUIButton.h"
#include "BsGUITexture.h"
#include "BsGUIPanel.h"
#include "BsGUISpace.h"
#include "BsDebug.h"
#include "BsBuiltinEditorResources.h"

using namespace std::placeholders;

namespace bs
{
	const Color GUIStatusBar::COLOR_INFO = Color(0.7f, 0.7f, 0.7f);
	const Color GUIStatusBar::COLOR_WARNING = Color(192 / 255.0f, 176 / 255.0f, 0.0f);
	const Color GUIStatusBar::COLOR_ERROR = Color(192 / 255.0f, 36 / 255.0f, 0.0f);

	GUIStatusBar::GUIStatusBar(const PrivatelyConstruct& dummy,
		const String& style, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions, style)
	{
		mPanel = GUIPanel::create();
		mBgPanel = GUIPanel::create(1);
		_registerChildElement(mPanel);
		_registerChildElement(mBgPanel);

		mBackground = GUITexture::create(GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(getGUIBackgroundTypeName()));
		mMessage = GUIButton::create(HString(L""), GUIOptions(GUIOption::flexibleWidth()), getSubStyleName(getGUIMessageTypeName()));
		mScene = GUILabel::create(HString(L"Scene: Unnamed"), GUIOptions(GUIOption::fixedWidth(150)));
		mProject = GUILabel::create(HString(L"Project: None"), GUIOptions(GUIOption::fixedWidth(200)));
		mCompiling = GUILabel::create(HString(L"Compiling..."), GUIOptions(GUIOption::fixedWidth(100)));

		GUILayoutY* vertLayout = mPanel->addNewElement<GUILayoutY>();
		vertLayout->addNewElement<GUIFixedSpace>(3);
		GUILayoutX* horzLayout = vertLayout->addNewElement<GUILayoutX>();

		horzLayout->addNewElement<GUIFixedSpace>(10);
		horzLayout->addElement(mMessage);
		horzLayout->addNewElement<GUIFlexibleSpace>();
		horzLayout->addElement(mScene);
		horzLayout->addNewElement<GUIFixedSpace>(10);
		horzLayout->addElement(mProject);
		horzLayout->addNewElement<GUIFixedSpace>(10);
		horzLayout->addElement(mCompiling);
		horzLayout->addNewElement<GUIFixedSpace>(10);

		mBgPanel->addElement(mBackground);
		mCompiling->setActive(false);

		mLogEntryAddedConn = gDebug().onLogModified.connect(std::bind(&GUIStatusBar::logModified, this));
		mMessageBtnPressedConn = mMessage->onClick.connect(std::bind(&GUIStatusBar::messageBtnClicked, this));
	}

	GUIStatusBar::~GUIStatusBar()
	{
		mLogEntryAddedConn.disconnect();
		mMessageBtnPressedConn.disconnect();
	}

	GUIStatusBar* GUIStatusBar::create(const GUIOptions& options, const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &getGUITypeName();

		return bs_new<GUIStatusBar>(PrivatelyConstruct(), *curStyle, GUIDimensions::create(options));
	}

	GUIStatusBar* GUIStatusBar::create(const String& style)
	{
		const String* curStyle = &style;
		if (*curStyle == StringUtil::BLANK)
			curStyle = &getGUITypeName();

		return bs_new<GUIStatusBar>(PrivatelyConstruct(), *curStyle, GUIDimensions::create());
	}

	void GUIStatusBar::setProject(const WString& name, bool modified)
	{
		WStringStream content;
		content << L"Project: ";

		if (name.size() > 20)
			content << name.substr(0, 20) << L"...";
		else
			content << name;

		if (modified)
			content << L"*";

		mProject->setContent(HString(content.str()));
	}

	void GUIStatusBar::setScene(const WString& name, bool modified)
	{
		WStringStream content;
		content << L"Scene: ";

		if (name.size() > 15)
			content << name.substr(0, 15) << L"...";
		else
			content << name;

		if (modified)
			content << L"*";

		mScene->setContent(HString(content.str()));
	}

	void GUIStatusBar::setIsCompiling(bool compiling)
	{
		mCompiling->setActive(compiling);
	}
	
	void GUIStatusBar::setTint(const Color& color)
	{
		mBackground->setTint(color);
		mMessage->setTint(color);
	}

	void GUIStatusBar::_updateLayoutInternal(const GUILayoutData& data)
	{
		mPanel->_setLayoutData(data);
		mPanel->_updateLayoutInternal(data);

		mBgPanel->_setLayoutData(data);
		mBgPanel->_updateLayoutInternal(data);
	}

	Vector2I GUIStatusBar::_getOptimalSize() const
	{
		return mBgPanel->_getOptimalSize();
	}
	
	void GUIStatusBar::styleUpdated()
	{
		mBackground->setStyle(getSubStyleName(getGUIBackgroundTypeName()));
		mMessage->setStyle(getSubStyleName(getGUIMessageTypeName()));
	}

	void GUIStatusBar::logModified()
	{
		LogEntry entry;
		if(!gDebug().getLog().getLastEntry(entry))
		{
			GUIContent messageContent(HString(L""));
			mMessage->setContent(messageContent);

			return;
		}

		HSpriteTexture iconTexture;
		Color textColor = COLOR_INFO;

		UINT32 logChannel = entry.getChannel();
		switch (logChannel)
		{
		case (UINT32)DebugChannel::Debug:
			iconTexture = BuiltinEditorResources::instance().getLogMessageIcon(LogMessageIcon::Info, 16, false);
			break;
		case (UINT32)DebugChannel::Warning:
		case (UINT32)DebugChannel::CompilerWarning:
			iconTexture = BuiltinEditorResources::instance().getLogMessageIcon(LogMessageIcon::Warning, 16, false);
			textColor = COLOR_WARNING;
			break;
		case (UINT32)DebugChannel::Error:
		case (UINT32)DebugChannel::CompilerError:
			iconTexture = BuiltinEditorResources::instance().getLogMessageIcon(LogMessageIcon::Error, 16, false);
			textColor = COLOR_ERROR;
			break;
		}

		WString message = toWString(entry.getMessage());
		size_t lfPos = message.find_first_of('\n');
		size_t crPos = message.find_first_of('\r');
		size_t newlinePos;

		if (lfPos != WString::npos)
		{
			if (crPos != WString::npos)
				newlinePos = std::min(lfPos, crPos);
			else
				newlinePos = lfPos;
		}
		else if (crPos != WString::npos)
			newlinePos = crPos;
		else
			newlinePos = -1;

		if (newlinePos == -1)
		{
			GUIContent messageContent(HString(message), iconTexture);
			mMessage->setContent(messageContent);
			mMessage->setTint(textColor);
		}
		else
		{
			WString firstLine = message.substr(0, newlinePos);

			GUIContent messageContent(HString(firstLine), iconTexture);
			mMessage->setContent(messageContent);
			mMessage->setTint(textColor);
		}		
	}

	void GUIStatusBar::messageBtnClicked()
	{
		onMessageClicked();
	}

	const String& GUIStatusBar::getGUITypeName()
	{
		static String TypeName = "GUIStatusBar";
		return TypeName;
	}

	const String& GUIStatusBar::getGUIBackgroundTypeName()
	{
		static String TypeName = "GUIStatusBarBg";
		return TypeName;
	}

	const String& GUIStatusBar::getGUIMessageTypeName()
	{
		static String TypeName = "GUIStatusBarMessage";
		return TypeName;
	}
}