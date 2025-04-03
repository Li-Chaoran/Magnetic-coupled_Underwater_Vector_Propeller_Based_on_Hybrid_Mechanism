using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Magnetic_coupling : MonoBehaviour
{
    public int limit_pwmrange = 50; // 最大限制为250
    private int start_pwm = 750;    // 初始值为750
    public int current_pwm = 750;   // 当前值为750
    // public GameObject chuanlianliangan1;
    private float speed1 = 0.0f;    // 用于显示孪生体运动的转动速度
    private int flag = 0;           // 状态标志符
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        int b = Input.GetKey(KeyCode.B) ? 1 : 0;                     // 获取按键状态
        int n = Input.GetKey(KeyCode.N) ? 1 : 0;                     // 获取按键状态
        // Debug.Log(speed1);
        if (b != 0)                                                  // B按钮下处理时间
        {
            if(flag ==2){speed1 = 0.0f;current_pwm = start_pwm;};    // 若从2状态变换过来，需要强制清零
            if(current_pwm < start_pwm+limit_pwmrange){              // 累加当前pwm值
                speed1 = speed1 + 0.1f;
                current_pwm = current_pwm + 25;
            } 
            transform.Rotate(0.0f, speed1, 0.0f, Space.Self);        // 孪生体显示转动
            flag = 1;                                                // 改变状态量
        }
        else if (n != 0)                                             // N按钮下处理时间
        {
            if(flag ==1){speed1 = 0.0f;current_pwm = start_pwm;}     // 若从1状态变换过来，强制清零
            if(current_pwm > start_pwm-limit_pwmrange){              // 累减当前pwm值
                speed1 = speed1 - 0.1f; 
                current_pwm = current_pwm - 25;
            }
            transform.Rotate(0.0f, speed1, 0.0f, Space.Self);        // 孪生体显示转动
            flag = 2;                                                // 改变状态量
        }
        else                                                         // 若没有按键按下则清零
        {
            flag = 0;
            current_pwm = start_pwm;              
            speed1 = 0.0f;
        }

        // 下面两行用于获取别的脚本的公共变量
        // float joint1 = chuanlianliangan1.GetComponent<joint1_rotation>().joint1;
        // Debug.Log(joint1);
    }
}
