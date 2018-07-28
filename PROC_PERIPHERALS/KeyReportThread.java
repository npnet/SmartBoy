package com.aawant.speaker.thread;

import java.io.InputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;

import com.aawant.speaker.MusApp;
import com.aawant.speaker.app.Sentence;
import com.aawant.speaker.app.SysContant;
import com.aawant.speaker.hardware.LedContralFacroty;
import com.aawant.speaker.hardware.XFService;
import com.aawant.speaker.manager.IXunFeiManagerListener;
import com.aawant.speaker.manager.XunFeiManager;
import com.aawant.speaker.model.eventbus.NetworkEvent;
import com.aawant.speaker.network.impl.RecRecognizeFactory;
import com.aawant.speaker.reciver.AlarmReceiver;
import com.aawant.speaker.service.PhoneStatusService;
import com.aawant.speaker.utils.SystemUtil;
import com.aawant.speaker.utils.ThreadUtil;
import com.ooframework.utils.LogUtil;

import android.annotation.SuppressLint;
import android.os.StrictMode;
import de.greenrobot.event.EventBus;

/**
 * 按键上报引
 * @date 2017-4-20
 */
public class KeyReportThread extends Thread{
	
	/**
	 * 1-f  触摸板按键（16个码） 
	 * p	物理按键短按、唤醒键
	 * q	物理按键长按
	 * v	语音唤醒 需要区分物理按键唤醒 语音唤醒在拨打电话时用于实现虚拟键盘功能
	 */
	
	public static AtomicBoolean isReport = new AtomicBoolean(false);
	
	public static AtomicLong isReportTime = new AtomicLong(System.currentTimeMillis());
	@SuppressLint("UseSparseArrays")
	private static Map<Integer, Integer> volumeToLed = new HashMap<Integer, Integer>();
	private static Map<String, Integer> changeMap = new HashMap<String, Integer>();
	private static String cacheStr = "";
	private static String keyStr = "13579bdfhjln";
	Socket socket = null;
	
	static {
		// 上报键值对于关系
		changeMap.put("1", 12);
		changeMap.put("3", 11);
		changeMap.put("5", 10);
		changeMap.put("7", 9);
		changeMap.put("9", 8);
		changeMap.put("b", 7);
		changeMap.put("d", 6);
		changeMap.put("f", 5);
		changeMap.put("h", 4);
		changeMap.put("j", 3);
		changeMap.put("l", 2);
		changeMap.put("n", 1);
		
		//音量与led灯光
		volumeToLed.put(0, 1);
		volumeToLed.put(1, 1);
		volumeToLed.put(2, 1);
		volumeToLed.put(3, 2);
		volumeToLed.put(4, 3);
		volumeToLed.put(5, 4);
		volumeToLed.put(6, 5);
		volumeToLed.put(7, 6);
		volumeToLed.put(8, 7);
		volumeToLed.put(9, 8);
		volumeToLed.put(10, 9);
		volumeToLed.put(11, 10);
		volumeToLed.put(12, 11);
		volumeToLed.put(13, 12);
		volumeToLed.put(14, 12);
		volumeToLed.put(15, 12);
		
	}
	boolean falg = false;
	@Override
	public void run() {
		super.run();
		int max = MusApp.getInstance().maxVolume;
		StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder().detectDiskReads().detectDiskWrites()
				.detectNetwork().penaltyLog().build());
		ThreadUtil.sleep(10000);
		InputStream is = null;
		try {
			socket = new Socket("127.0.0.1", 9898);
			socket.setSoTimeout(0);
			byte[] buffer = null;
			int size = 0;
			String temp = "";
			LogUtil.d("OO", "KeyReportThread start");
			is = socket.getInputStream();
			long time = System.currentTimeMillis();
			while (true) {
				try {
					temp = null;
					buffer = new byte[128];
					size = is.read(buffer);
					temp = new String(buffer, 0, size).substring(0, 1);
					if (RecRecognizeFactory.startNetWork.get() && !RecRecognizeFactory.runState.get()) {
						LogUtil.d("filter", " >>>>continue>>>RecRecognizeFactory.startNetWork.get() && !RecRecognizeFactory.runState.get()");
						continue;
					}
					
					// 除了音量键、一秒内限制一次按键上报
					if ("p".equals(temp) || "q".equals(temp) || "v".equals(temp)) {
						if (System.currentTimeMillis() - time > 1500) {
							time = System.currentTimeMillis();
						} else {
							LogUtil.d("OO", ">>>filter key:" + temp);
							continue;
						}
					}
					
					LogUtil.d("OO", " >>>>" + temp.substring(0, 1));
					
					// 先处理音量
					if (!"p".equals(temp) && !"q".equals(temp) && !"v".equals(temp)) {
						if (PhoneStatusService.afterPhoneToFilterVoice != 0 && (System.currentTimeMillis()-PhoneStatusService.afterPhoneToFilterVoice) < 6000) {
							LogUtil.d("OO", " >>>>Continue By afterPhoneToFilterVoice");
							continue;
						}
						
						if (SysContant.isPlayPhone.get() || SysContant.isRingPhone.get()) {
							LogUtil.d("filter", " >>>>Continue By Phone");
							continue;
						}
						
						if (RecRecognizeFactory.runState.get()) {
							LogUtil.d("filter", " >>>>continue>>>RecRecognizeFactory.runState.get()" + RecRecognizeFactory.runState.get());
							continue;
						}
						// 过滤异常上报的键值
						if (!keyStr.contains(temp)) {
							continue;
						}
						
						if (SysContant.isSleep.get()) {
							continue;
						}
						
						// 超过一秒没有按键上报清空缓存区
						if ((System.currentTimeMillis() - isReportTime.get()) > 1000) {
							LogUtil.d("OO", "cacheStr");
							cacheStr = "";
						}
						isReportTime.set(System.currentTimeMillis());
						isReport.set(true);
						
						if (2 == cacheStr.length()) {
							cacheStr = cacheStr.substring(1) + temp;
						} else {
							cacheStr = cacheStr + temp;
						}
						int current = MusApp.getInstance().currentSound();	
						
						LogUtil.d("OO", "volume=" + current + "|cacheStr=" + cacheStr);
						
						int result = volumChange2(cacheStr);
						// 音量控制过程
						if (1 == result || 2 == result) {
							SysContant.isInVoiceLed.set(true);
						}
						if (1==result) {
							if (current > 1) {
								current = current - 1;
								LogUtil.d("OO", "1current=" + current);
								MusApp.getInstance().soundSetValue(current);
								//音量调节灯光
								LedContralFacroty.getInstance().volumeStatusLed(volumeToLed.get(current));
							} else {
								LogUtil.d("OO", "2current=" + current);
								LedContralFacroty.getInstance().volumeStatusLed(volumeToLed.get(current));
							}
							if (!ProcessThread.getInstance().isPlaying() && !AlarmReceiver.isInAlarm.get()) { 
								MusApp.getInstance().playTipsMap3(19);
							}
						} else if (2== result) {
							if (max > current) {
								current = current + 1;
								if (current > 13) {
									MusApp.getInstance().soundSetValue(12);
								} else {
									MusApp.getInstance().soundSetValue(current);
								}
								LogUtil.d("OO", "3current=" + current);
								
								LedContralFacroty.getInstance().volumeStatusLed(volumeToLed.get(current));
							} else {
								LogUtil.d("OO", "4current=" + current);
								LedContralFacroty.getInstance().volumeStatusLed(volumeToLed.get(current));
							}
							if (!ProcessThread.getInstance().isPlaying() && !AlarmReceiver.isInAlarm.get()) {
								MusApp.getInstance().playTipsMap3(19);
							}
						} else if (0 == result) {
						} 
						if (!SysContant.isInBinding.get() || !SysContant.wifiNetWorkState.get()) {
							LigthEffThread.errorNetTime = System.currentTimeMillis();
						}
						continue;
					}
					
					// 音箱初始化完成后才允许按键上报
					if (!SysContant.initSuccess.get()) {
						LogUtil.d("filter", " >>>>continue>>>SysContant.initSuccess.get()" + SysContant.initSuccess.get());
						continue;
					}
					
					if (SysContant.changeData.get()) {
						LogUtil.d("filter", " >>>>continue>>>SysContant.changeData.get()" + SysContant.changeData.get());
						continue;
					}
					if (SysContant.changeNet.get()) {
						LogUtil.d("filter", " >>>>continue>>>SysContant.changeNet.get()" + SysContant.changeNet.get());
						continue;
					}
					if ("p".equals(temp)) {
						if (PhoneStatusService.STATUS_RINGTOPLAY == PhoneStatusService.isRingToPlay.get()) {
							LogUtil.d("filter", " >>>>continue>>>PhoneStatusService.isRingToPlay.get()");
							continue;
						}
						if (SysContant.isPlayPhone.get()) { // 处理打电话物理按键唤醒唤醒键作为挂机
							LogUtil.d("OO", "endCall");
							SystemUtil.endCall();
							ProcessThread.virtualKey = false;
							ProcessThread.virtualKeyRing = false;
							continue;
						} else if (SysContant.isRingPhone.get()) {
							PhoneStatusService.isRingToPlay.set(PhoneStatusService.STATUS_RINGTOPLAY);
							ExectorsThread.exectorCurrnet(new Runnable() {
								public void run() {
									XunFeiManager.getInstance().spreak(Sentence.PICKCALL, new IXunFeiManagerListener.IXunFeiSpeakerListener() {
										@Override
										public void onSpeakerFinish() {}
										@Override
										public void onSpeakerComplete() {
											SystemUtil.acceptCall();
											LogUtil.d("OO", "answerRingingCall");
										}
									});
								}
							});
							continue;
						} else if (AlarmReceiver.isInAlarm.get()) {// 处于闹钟时刻
							LogUtil.d("OO", "Cancel Alarm");
							AlarmReceiver.isInAlarm.set(false);
							ProcessThread.getInstance().stopListening();
							MusApp.getInstance().pauseTipsMp3(MusApp.getInstance().getPlayingTipsMp3Id());
							XunFeiManager.getInstance().spreak(Sentence.ALARMCANCEL, new IXunFeiManagerListener.IXunFeiSpeakerListener() {
								@Override
								public void onSpeakerFinish() {}
								@Override
								public void onSpeakerComplete() {
								}
							});
							continue;
						} else if (RecRecognizeFactory.tryConnect.get()) {
							if (!XunFeiManager.getInstance().isSpreak()) {
								XunFeiManager.getInstance().spreak(Sentence.TRYCONNECTNET);
							}
						} else if (RecRecognizeFactory.runState.get()) { // 处于连网模式
							if (!XunFeiManager.getInstance().isSpreak()) {
								RecRecognizeFactory.getInstance().destory();
								LogUtil.d("OO", "Recognize destory");
							} else {
								LogUtil.d("OO", "filter netconfig destory beacuse spreaking");
							}
						} else {
							if (SysContant.wifiNetWorkState.get()) {
								// 提示未绑定
								if (SysContant.isBinding.get()) {
									SysContant.isWake.set(true);
									SysContant.isSleep.set(false);/**/
								} else {
									if (SysContant.isInBinding.get()) {
										XunFeiManager.getInstance().spreak(Sentence.INBINDING);
									} else {
										LigthEffThread.errorNetTime = System.currentTimeMillis();
										LedContralFacroty.getInstance().errorNetLed();
										if (!XunFeiManager.getInstance().isSpreak()) {
											XunFeiManager.getInstance().spreak(Sentence.BINDINGBOX);
										}
									}
								}
							} else {
								if (SysContant.isSleep.get()) {
									SysContant.isSleep.set(false);
								}
								LigthEffThread.errorNetTime = System.currentTimeMillis();
								LedContralFacroty.getInstance().errorNetLed();
								if (!XunFeiManager.getInstance().isSpreak()) {
									if (!RecRecognizeFactory.exitNetWork.get()) {
										XunFeiManager.getInstance().spreak(Sentence.ERRORNET);
									}
								}
							}
						} 
					} else if ("q".equals(temp)) {
						if (SysContant.isPlayPhone.get() || SysContant.isRingPhone.get()) {
							LogUtil.d("filter",
									"filter netconfig isPlayPhone=" + SysContant.isPlayPhone.get() + "|isRingPhone="
											+ SysContant.isRingPhone.get());
							continue;
						} 
						if (AlarmReceiver.isInAlarm.get()) {
							AlarmReceiver.isInAlarm.set(false);
							MusApp.getInstance().pauseTipsMp3(MusApp.getInstance().getPlayingTipsMp3Id());
							ProcessThread.getInstance().stopByWake();
							SysContant.runState.set(false);
							SysContant.ledQuickTrans.set(false);
							SysContant.ledSlowTrans.set(false);
							SysContant.isSleep.set(false);
							SysContant.isInBinding.set(false);
							EventBus.getDefault().post(new NetworkEvent(NetworkEvent.NETWORK_CONFIG));
						}
						if (!RecRecognizeFactory.runState.get()) {
							ProcessThread.getInstance().stopByWake();
							SysContant.runState.set(false);
							SysContant.ledQuickTrans.set(false);
							SysContant.ledSlowTrans.set(false);
							SysContant.isSleep.set(false);
							SysContant.isInBinding.set(false);
							EventBus.getDefault().post(new NetworkEvent(NetworkEvent.NETWORK_CONFIG));
						} else {
							LogUtil.d("OO", "Already In Network Configure!");
						}
					} else if ("v".equals(temp)) {
						// 处于闹钟时刻
						if (AlarmReceiver.isInAlarm.get()) {
							LogUtil.d("OO", "Cancel Alarm");
							AlarmReceiver.isInAlarm.set(false);
							ProcessThread.getInstance().stopListening();
							MusApp.getInstance().pauseTipsMp3(MusApp.getInstance().getPlayingTipsMp3Id());
							XunFeiManager.getInstance().spreak(Sentence.ALARMCANCEL, new IXunFeiManagerListener.IXunFeiSpeakerListener() {
								@Override
								public void onSpeakerFinish() {}
								@Override
								public void onSpeakerComplete() {
								}
							});
							continue;
						} else if (SysContant.isSleep.get()) {
							// 休眠过滤语音唤醒
						} else if (RecRecognizeFactory.runState.get()) {
							// 联网时过滤唤醒
						} else {
							if (SysContant.wifiNetWorkState.get()) {
								// 提示未绑定
								if (SysContant.isBinding.get()) {
									SysContant.isWakeByVoice.set(true);
									SysContant.isSleep.set(false);
								} else {
									if (SysContant.isInBinding.get()) {
										XunFeiManager.getInstance().spreak(Sentence.INBINDING);
									} else {
										LigthEffThread.errorNetTime = System.currentTimeMillis();
										LedContralFacroty.getInstance().errorNetLed();
										if (!XunFeiManager.getInstance().isSpreak()) {
											XunFeiManager.getInstance().spreak(Sentence.BINDINGBOX);
										}
									}
								}
							} else {
								if (PhoneStatusService.STATUS_RINGTOPLAY == PhoneStatusService.isRingToPlay.get()) {
									LogUtil.d("filter", " >>>>continue>>>PhoneStatusService.isRingToPlay.get()" + PhoneStatusService.isRingToPlay.get());
									continue;
								}
								if (SysContant.isPlayPhone.get()) {
									SystemUtil.endCall();
									SysContant.isPlayPhone.set(false);
									SysContant.ledSlowTrans.set(false);
									continue;
								} else if (SysContant.isRingPhone.get()) {
									PhoneStatusService.isRingToPlay.set(PhoneStatusService.STATUS_RINGTOPLAY);
									SysContant.ledSlowTrans.set(false);
									ExectorsThread.exectorCurrnet(new Runnable() {
										public void run() {
											XunFeiManager.getInstance().spreak(Sentence.PICKCALL, new IXunFeiManagerListener.IXunFeiSpeakerListener() {
												@Override
												public void onSpeakerFinish() {}
												@Override
												public void onSpeakerComplete() {
													SystemUtil.acceptCall();
													LogUtil.d("OO", "answerRingingCall");
												}
											});
										}
									});
									continue;
								}
								LigthEffThread.errorNetTime = System.currentTimeMillis();
								XFService.getInstance().resetXf();
								LedContralFacroty.getInstance().errorNetLed();
								if (!XunFeiManager.getInstance().isSpreak()) {
									XunFeiManager.getInstance().spreak(Sentence.ERRORNET);
								}
							}
						} 
						
					}
				} catch (Exception e) {
					if (socket.isClosed()) {
						try {
							socket = new Socket("127.0.0.1", 9898);
						} catch (Exception e1) {
							e1.printStackTrace();
						}
					}
					LogUtil.e("OO", e);
				}
			}
		} catch (Throwable e) {
			LogUtil.e("OO", e);
			e.printStackTrace();
			if (null != socket && socket.isClosed()) {
				try {
					socket = new Socket("127.0.0.1", 9898);
				} catch (Exception e1) {
					e1.printStackTrace();
				}
			} else {
				if (null == socket) {
					LogUtil.d("OO", "restart keyReportThread!");
					new KeyReportThread().start();
				}
			}
		}
	}
	
	
	/**
	 * 音量加减
	 * @return 2 加 1 减 
	 */
	private int volumChange2(String str) {
		try {
			int i = 0;
			if (str.length() != 2) {
				return 0;
			}
			int a = changeMap.get(str.substring(0, 1));
			int b = changeMap.get(str.substring(1, 2));
			// 音量减
			if (a > b) {
				if ((a - b) <= 4) {
					return 1;
				}
			}
			// 音量加
			if (b > a) {
				if ((b - a) <= 4) {
					return 2;
				}
			}
			return i;
		
		} catch (Exception e) {
			LogUtil.e("OO", e);
		}
		return 0;
	}
	
	public int volumChange(String str) {
		int i = 0;
		try {
			if (str.length() != 3) {
				return 0;
			}
			int a = changeMap.get(str.substring(0, 1));
			int b = changeMap.get(str.substring(1, 2));
			int c = changeMap.get(str.substring(2, 3));
			if (a > b && b > c) {
				return 1;
			}
			if (a < b && b < c) {
				return 2;
			}
			
		} catch (Exception e) {
			LogUtil.e("OO", e);
		}
		return i;
	}
}
