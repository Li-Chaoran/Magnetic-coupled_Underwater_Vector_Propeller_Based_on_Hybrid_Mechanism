using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CurrentPwm_Show : MonoBehaviour
{
    public Text pwm_show;
    public GameObject magnetic_coupling_show;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        pwm_show.text = "Current_PWM:" + magnetic_coupling_show.GetComponent<Magnetic_coupling>().current_pwm;
    }
}
