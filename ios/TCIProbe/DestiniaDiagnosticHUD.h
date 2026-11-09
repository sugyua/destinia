Command exited with code 0.

Command output:

      if (_framePresentationPending.exchange(true)) return;
      UIImage *image=DestiniaImageFromRGB565(pixels.data());
      if (!image) {
          _framePresentationPending.store(false);
          [self showStatus:@"Erro ao converter RGB565"];
          return;
      }
      dispatch_async(dispatch_get_main_queue(), ^{
          self->_controller.gameView.image=image;
          self->_controller.statusLabel.hidden=
              !self->_showMetrics.load() && !self->_lastProfile;
          self->_framePresentationPending.store(false);
      });
  }
  
> - (void)armTimerNs:(uint64_t)delayNs {
      if (!_timer) {
          _timer=dispatch_source_create(
              DISPATCH_SOURCE_TYPE_TIMER,0,0,_gameQueue);
          __weak AppDelegate *weakSelf=self;
          dispatch_source_set_event_handler(_timer,^{
              AppDelegate *owner=weakSelf;
              if (owner) { @autoreleasepool { [owner gameTick]; } }
          });
          dispatch_resume(_timer);
      }
      const uint64_t ns=std::max<uint64_t>(
          NSEC_PER_MSEC,std::min<uint64_t>(delayNs,1000ULL*NSEC_PER_MSEC));
      // Tighten the 5ms leeway of 0.3.8 to 0.5ms. The precise 62.5ms
      // start-to-start target is calculated in nextFrameWait().
      _timerDeadline=std::chrono::steady_clock::now()+
          std::chrono::nanoseconds(ns);
      _hasTimerDeadline=true;
      dispatch_source_set_timer(
          _timer,dispatch_time(DISPATCH_TIME_NOW,static_cast<int64_t>(ns)),
          DISPATCH_TIME_FOREVER,500000ULL);
  }
  
  - (void)armTimer:(uint32_t)delay {
>     [self armTimerNs:static_cast<uint64_t>(delay)*NSEC_PER_MSEC];
  }
  
  - (void)gameTick {
      if (!_ready) return;
      if (_paused.load()) {
          [self armTimer:250];
          _hasPreviousFrame=false;
          _hasTimerDeadline=false;
          return;
      }
  
      const auto tickStart=std::chrono::steady_clock::now();
      const double timerLateMs=_hasTimerDeadline
          ? std::max(0.0,std::chrono::duration<double,std::milli>(
                tickStart-_timerDeadline).count())
          _totalFrameMs=0;
          _maxFrameMs=0;
          _peakArmMs=0;
          _peakTimerLateMs=0;
      }
      if (nextMs==0) {
          _ready=false;
          [self showStatus:@"Destinia encerrou a execucao."];
          return;
      }
      // Never accelerate gameplay beyond the original 16Hz logic cadence.
      // Honor larger guest-requested delays, including loading/transitions.
      const auto wait=destinia::nextFrameWait(
          tickStart,std::chrono::steady_clock::now(),nextMs);
      _previousGuestDelayMs=nextMs;
>     [self armTimerNs:static_cast<uint64_t>(wait.count())];
  }
  
  - (BOOL)application:(UIApplication *)application
      didFinishLaunchingWithOptions:(NSDictionary *)options {
      _paused.store(false);
      _showMetrics.store(true);
      _framePresentationPending.store(false);
      _reusablePixels.resize(kFrameBytes);
      _ready=false;
      _frameIndex=0;
      _measureStarted=std::chrono::steady_clock::now();
      _measureFrames=0;
      _totalArmMs=0;
      _totalPreparationMs=0;
      _totalFrameMs=0;
                      [path fileSystemRepresentation],
                      output,sizeof(output),
                      pixels.data(),pixels.size())) {
                  NSString *error=[NSString stringWithFormat:@"%s",output];
                  [self showStatus:error];
                  return;
              }
              NSLog(@"Destinia iOS: %s",output);
              _ready=true;
              _measureStarted=std::chrono::steady_clock::now();
              _measureFrames=0;
              _totalArmMs=0;
              _totalPreparationMs=0;
              _frameIndex=4;
              [self showPixels:pixels frameNum:_frameIndex];
>             [self armTimerNs:62500000ULL];
          }
      });
      return YES;
  }
  
  - (void)applicationWillResignActive:(UIApplication *)application {
      _paused.store(true);
  }
  - (void)applicationDidBecomeActive:(UIApplication *)application {
      _paused.store(false);
  }
  - (void)applicationWillTerminate:(UIApplication *)application {
      _paused.store(true);
      dispatch_async(_gameQueue,^{ destinia_game_close(); });
  }


