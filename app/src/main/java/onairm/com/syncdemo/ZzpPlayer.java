package onairm.com.syncdemo;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by Edison on 2017/11/28.
 */

public class ZzpPlayer extends SurfaceView implements SurfaceHolder.Callback{
    static {
        System.loadLibrary("native-lib");
    }
    public ZzpPlayer(Context context) {
        this(context,null,0);
    }

    public ZzpPlayer(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public ZzpPlayer(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        play("/sdcard/input.mp4",surfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }
    public native void play(String path, Surface surface);
}
