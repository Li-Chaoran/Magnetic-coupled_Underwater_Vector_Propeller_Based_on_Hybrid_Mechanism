using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.WSA;
using System;

public class Moving_plate : MonoBehaviour
{
    // private float joint1 = 90.0f;
    // private float joint2 = 0.0f;
    public GameObject qudonggan1;
    public GameObject qudonggan2;
    public GameObject qudonggan3;
    public float y_rotate_sum = 0.0f;
    public float z_rotate_sum = 0.0f;
    private float sum = 0.0f;
    // Start is called before the first frame update
    void Start()
    {
        transform.SetPositionAndRotation(new Vector3(-0.9578352f,0.9859999f,0.0f), Quaternion.Euler(57.853f, 0.0f, 90.0f));
    }

    // Update is called once per frame
    void Update()
    {
        float h = Input.GetAxis("Vertical");
        float m = Input.GetAxis("Horizontal");
        sum = (float)Math.Sqrt(Math.Pow(z_rotate_sum, 2) + Math.Pow(y_rotate_sum, 2));
        if(h != 0){           // 限制上下键旋转角度，防止连接出现断裂，这里限制在了顶角为70度的圆锥体内
            if (0.0f<=sum && sum<30.0f){
                transform.Rotate(new Vector3(0.0f, 0.0f, 1.0f), -h*0.02f, Space.World);
                z_rotate_sum = z_rotate_sum + h*0.02f; 
            }
            else if(z_rotate_sum<0.0f && h>0.0f){
                transform.Rotate(new Vector3(0.0f, 0.0f, 1.0f), -h*0.02f, Space.World);
                z_rotate_sum = z_rotate_sum + h*0.02f; 
            }
            else if(z_rotate_sum>0.0f && h<0.0f){
                transform.Rotate(new Vector3(0.0f, 0.0f, 1.0f), -h*0.02f, Space.World);
                z_rotate_sum = z_rotate_sum + h*0.02f; 
            }
        };
        if(m != 0){          // 限制左右键旋转角度，防止连接出现断裂
            if( 0.0f<=sum && sum<30.0f){
                transform.Rotate(new Vector3(0.0f, 1.0f, 0.0f), -m*0.02f, Space.World);
                y_rotate_sum = y_rotate_sum + m*0.02f;
            }
            else if(y_rotate_sum<0.0f && m>0.0f){
                transform.Rotate(new Vector3(0.0f, 1.0f, 0.0f), -m*0.02f, Space.World);
                y_rotate_sum = y_rotate_sum + m*0.02f;
            }
            else if(y_rotate_sum>0.0f && m<0.0f){
                transform.Rotate(new Vector3(0.0f, 1.0f, 0.0f), -m*0.02f, Space.World);
                y_rotate_sum = y_rotate_sum + m*0.02f;
            }
        };
    }
    
}
