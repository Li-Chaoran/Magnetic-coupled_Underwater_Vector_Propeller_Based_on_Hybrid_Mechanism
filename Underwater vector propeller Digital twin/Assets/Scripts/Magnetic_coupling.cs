using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Magnetic_coupling : MonoBehaviour
{
    public int limit_pwmrange = 50; // �������Ϊ250
    private int start_pwm = 750;    // ��ʼֵΪ750
    public int current_pwm = 750;   // ��ǰֵΪ750
    // public GameObject chuanlianliangan1;
    private float speed1 = 0.0f;    // ������ʾ�������˶���ת���ٶ�
    private int flag = 0;           // ״̬��־��
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        int b = Input.GetKey(KeyCode.B) ? 1 : 0;                     // ��ȡ����״̬
        int n = Input.GetKey(KeyCode.N) ? 1 : 0;                     // ��ȡ����״̬
        // Debug.Log(speed1);
        if (b != 0)                                                  // B��ť�´���ʱ��
        {
            if(flag ==2){speed1 = 0.0f;current_pwm = start_pwm;};    // ����2״̬�任��������Ҫǿ������
            if(current_pwm < start_pwm+limit_pwmrange){              // �ۼӵ�ǰpwmֵ
                speed1 = speed1 + 0.1f;
                current_pwm = current_pwm + 25;
            } 
            transform.Rotate(0.0f, speed1, 0.0f, Space.Self);        // ��������ʾת��
            flag = 1;                                                // �ı�״̬��
        }
        else if (n != 0)                                             // N��ť�´���ʱ��
        {
            if(flag ==1){speed1 = 0.0f;current_pwm = start_pwm;}     // ����1״̬�任������ǿ������
            if(current_pwm > start_pwm-limit_pwmrange){              // �ۼ���ǰpwmֵ
                speed1 = speed1 - 0.1f; 
                current_pwm = current_pwm - 25;
            }
            transform.Rotate(0.0f, speed1, 0.0f, Space.Self);        // ��������ʾת��
            flag = 2;                                                // �ı�״̬��
        }
        else                                                         // ��û�а�������������
        {
            flag = 0;
            current_pwm = start_pwm;              
            speed1 = 0.0f;
        }

        // �����������ڻ�ȡ��Ľű��Ĺ�������
        // float joint1 = chuanlianliangan1.GetComponent<joint1_rotation>().joint1;
        // Debug.Log(joint1);
    }
}
