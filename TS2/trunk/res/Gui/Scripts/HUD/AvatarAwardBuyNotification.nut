// Avatar Award Notification

// Resources

sigimport "gui/scripts/hud/achievementbuynotification.nut"

sigexport function CanvasCreateAchievementBuyNotification( cppObj )
{
	return ::BuyNotificationMenuStack( cppObj, false )
}